#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include "server.h"
#include "queue.h"


pthread_mutex_t lock;
bool done = false;

int thread_count = 0;
struct thread_management {
    pthread_t thread_id;
    bool exited;
    bool available;
} thread_man[MAX_THREADS];

struct queue *conn_queue;


void *client_handler(void *parameters) {
    HANDLER_PARAMS_PTR p = (HANDLER_PARAMS_PTR)parameters;
    int idx = p->slot;
    bool *exited = &(thread_man[idx].exited);
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&p->client_address;
    char *ip = inet_ntoa(addr_in->sin_addr);

    printf("New Thread (slot %d) - thread_id %ld (Total %d)\n", idx, pthread_self(), get_thread_count_safe());
    printf("\tNew connection: client ip %s, port %d\n", ip, addr_in->sin_port);

    /* Do thread-specific work here */
    sleep(20);
    /* End thread-specific work here */

    close(p->connfd);

    printf("Exiting Thread idx %d - thread_id %ld\n", idx, thread_man[idx].thread_id);

    free(parameters);

    *exited = true;
    dec_thread_count_safe();
    pthread_exit(NULL);
}


int main(int argc, int *argv) {
    struct sockaddr_in server_address, client_address;
    int sockfd, connfd, client_size = sizeof(client_address), port = PORT;
    int i = 0, slot = -1;


    printf("Server starts!\n\n");

    if (!initialise()) {
        exit(1);
    }


    /* 1. Create socket */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed!\n");
        exit(2);
    }

    bzero(&server_address, sizeof(server_address));


    /* 2. Bind socket to server address */

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (bind(sockfd, (SOCKADDR_PTR)&server_address, sizeof(server_address)) != 0) {
        printf("Socket bind failed!\n");
        exit(3);
    }

    fcntl(sockfd, F_SETFL, O_NONBLOCK);


    /* 3. Socket to passive mode, listen for connections */

    if (listen(sockfd, 5) != 0) {
        printf("Socket listen failed!\n");
        exit(4);
    }

    printf("Listening on port %d\n", port);


    /* 4. Accept connections from client */

    while (!done) {
        connfd = accept(sockfd, (SOCKADDR_PTR)&client_address, &client_size);

        /* 5. Add the connection to the queue */

        if (connfd > 0) {
            HANDLER_PARAMS_PTR gci = get_client_info(-1, connfd, client_address);
            if (enqueue(conn_queue, gci)) {
                printf("Stored client connection (%d)\n", queue_size(conn_queue));
            } else {
                printf("couldn\'t store connection... Closing\n");
                close(connfd);
            }
        }

        /* 5a. Get next connection and start a thread */

        if ((queue_size(conn_queue) > 0) && (slot = get_next_slot()) > -1) {
            printf("\tStarting thread in slot %d\n", slot);
            void *data = dequeue(conn_queue);
            HANDLER_PARAMS_PTR gci = (HANDLER_PARAMS_PTR)data;
            gci->slot = slot;
            create_new_thread(gci);
        }
    }


    /* 6. Wait for all threads to finish */

    for (i = 0; i < MAX_THREADS; i++) {
        if (!thread_man[i].available) {
            pthread_join(thread_man[i].thread_id, NULL);
            thread_man[i].available = true;
        }
    }

    process_queue(conn_queue, cleanup_socket_connections);
    destroy_queue(conn_queue);

    /* 7. Clean up server socket */

    close(sockfd);

    return 0;
}


void cleanup_socket_connections(void *p) {
    HANDLER_PARAMS_PTR gci = (HANDLER_PARAMS_PTR)p;
    close(gci->connfd);
}


void create_new_thread(HANDLER_PARAMS_PTR gci) {
    int slot = gci->slot;
    thread_man[slot].exited = false;
    thread_man[slot].available = false;
    inc_thread_count_safe();
    pthread_create(&thread_man[slot].thread_id, NULL, client_handler, gci);
}


HANDLER_PARAMS_PTR get_client_info(int slot, int connfd, struct sockaddr_in client_address) {
    HANDLER_PARAMS_PTR params = (HANDLER_PARAMS_PTR)malloc(sizeof(struct handler_params));
    if (params != NULL) {
        params->slot = slot;
        params->connfd = connfd;
        params->client_address = client_address;
    }
    return params;
}


int get_next_slot() {
    int i;

    for (i = 0; i < MAX_THREADS; i++) {
        if ((thread_man[i].available) || (thread_man[i].exited)) {
            return i;
        }
    }

    return -1;
}


bool initialise() {
    int i;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex initialisation failed!");
        return false;
    }

    for (i = 0; i < MAX_THREADS; i++) {
        thread_man[i].exited = false;
        thread_man[i].available = true;
    }

    conn_queue = create_new_queue();

    return true;
}


int get_thread_count_safe() {
    int count = 0;
    pthread_mutex_lock(&lock);
    count = thread_count;
    pthread_mutex_unlock(&lock);
    return count;
}


void inc_thread_count_safe() {
    pthread_mutex_lock(&lock);
    thread_count++;
    pthread_mutex_unlock(&lock);
}


void dec_thread_count_safe() {
    pthread_mutex_lock(&lock);
    thread_count--;
    pthread_mutex_unlock(&lock);
}
