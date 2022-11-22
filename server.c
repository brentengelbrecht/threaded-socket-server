#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "server.h"
#include "queue.h"
#include "key_list.h"

#define THREAD_MANAGEMENT struct thread_management
#define THREAD_MANAGEMENT_PTR struct thread_management *


THREAD_MANAGEMENT {
    int key;
    pthread_t thread_id;
};

QUEUE_PTR conn_queue = NULL;
KEY_LIST_PTR thread_list = NULL;

pthread_mutex_t lock;
bool done = false;


void *client_handler(void *parameters) {
    THREAD_MANAGEMENT_PTR t_data;
    KEY_LIST_NODE_PTR t_ptr;

    HANDLER_PARAMS_PTR p = (HANDLER_PARAMS_PTR)parameters;
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&p->client_address;
    char *ip = inet_ntoa(addr_in->sin_addr);

    t_ptr = (KEY_LIST_NODE_PTR)find_key_list_node(thread_list, p->thread_key);
    if (t_ptr != NULL) {
        t_data = (THREAD_MANAGEMENT_PTR)t_ptr->data;
        printf("\tNew Thread (key %d) - thread_id %ld (Total threads %d)\n", t_data->key, pthread_self(), get_key_list_size(thread_list));
        printf("\t\tNew connection: client ip %s, port %d\n", ip, addr_in->sin_port);
    }

    /* Do thread-specific work here */
    sleep(SLEEP);
    /* End thread-specific work here */

    close(p->connfd);

    printf("Exiting Thread (key %d) - thread_id %ld\n", t_data->key, t_data->thread_id);

    free(parameters);

    remove_key_list_node(thread_list, t_data->key);

    //pthread_detach(pthread_self());
    pthread_exit(NULL);
}


int main(int argc, int *argv) {
    signal(SIGINT, signal_handler);

    struct sockaddr_in server_address, client_address;
    int sockfd, connfd, client_size = 0, port = PORT;
    int i = 0;


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
            HANDLER_PARAMS_PTR gci = get_client_info(connfd, client_address);
            if (enqueue(conn_queue, gci)) {
                printf("Stored client connection (%d)\n", queue_size(conn_queue));
            } else {
                printf("Couldn\'t store connection... Closing\n");
                close(connfd);
            }
        }

        /* 5a. Get next connection and start a thread */

        if (!is_key_list_full(thread_list) && queue_size(conn_queue) > 0) {
            HANDLER_PARAMS_PTR client_conn = (HANDLER_PARAMS_PTR)dequeue(conn_queue);

            THREAD_MANAGEMENT_PTR t = (THREAD_MANAGEMENT_PTR)malloc(sizeof(THREAD_MANAGEMENT));
            if (t == NULL) {
                printf("Memory allocation for thread management node failed!\n");
            } else {
                int key = add_key_list_node(thread_list, t);
                t->key = client_conn->thread_key = key;
                printf("Starting thread (key %d)\n", key);
                pthread_create(&t->thread_id, NULL, client_handler, client_conn);
            }
        }
    }


    /* 6. Close any open client sockets, wait for all threads to finish, */

    printf("\nCleaning up sockets\n");
    process_queue(conn_queue, cleanup_socket_connections);
    destroy_queue(conn_queue);

    printf("Cleaning up threads\n");
    process_key_list(thread_list, cleanup_threads);
    destroy_key_list(thread_list);


    /* 7. Clean up server socket */

    close(sockfd);

    printf("\nServer stops.\n");
    return 0;
}


bool cleanup_socket_connections(void *p) {
    HANDLER_PARAMS_PTR client_conn = (HANDLER_PARAMS_PTR)p;
    if (client_conn->connfd > 0) {
        close(client_conn->connfd);
    }
    return false;
}


bool cleanup_threads(void *p) {
    THREAD_MANAGEMENT_PTR t_ptr = (THREAD_MANAGEMENT_PTR)p;
    pthread_join(t_ptr->thread_id, NULL);
    return false;
}


HANDLER_PARAMS_PTR get_client_info(int connfd, struct sockaddr_in client_address) {
    HANDLER_PARAMS_PTR p = (HANDLER_PARAMS_PTR)malloc(sizeof(HANDLER_PARAMS));
    if (p != NULL) {
        p->connfd = connfd;
        p->client_address = client_address;
        p->start = time(NULL);
    }
    return p;
}


bool initialise() {
    int i;

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex initialisation failed!");
        return false;
    }

    thread_list = create_new_key_list(MAX_THREADS, &lock);

    conn_queue = create_new_queue();

    return true;
}


void signal_handler(int signal) {
    done = true;
}
