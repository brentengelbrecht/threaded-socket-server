#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include "server.h"


void *client_handler(void *parameters) {
    HANDLER_PARAMS_PTR p = (HANDLER_PARAMS_PTR)parameters;
    int idx = p->slot;
    bool *exited = &(thread_man[idx].exited);
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&p->client_address;
    char *ip = inet_ntoa(addr_in->sin_addr);

    printf("New Thread (slot %d) - thread_id %ld (Total %d)\n", idx, pthread_self(), get_thread_count_safe());
    printf("\tNew connection: client ip %s, port %d\n", ip, addr_in->sin_port);

    sleep(20);

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


    /* 3. socket to passive mode, list for connections */

    if (listen(sockfd, 5) != 0) {
        printf("Socket listen failed!\n");
        exit(4);
    }

    printf("Listening on port %d\n", port);


    /* 4. Accept connections from client */

    while (!done) {
        connfd = accept(sockfd, (SOCKADDR_PTR)&client_address, &client_size);
        if (connfd < 0) {
            printf("Server accept failed!\n");
            done = true;
            continue;
        }

        /* 5. Hand-off to a new thread */

        if ((slot = get_next_slot()) == -1) {
            printf("Maxed out threads (%d)... No more connections\n", MAX_THREADS);
            close(connfd);
        } else {
            printf("\tStarting thread in slot %d\n", slot);

            HANDLER_PARAMS_PTR gci = get_client_info(slot, connfd, client_address);
            if (gci == NULL) {
                printf("Memory allocation failed! Closing connection...\n");
                close(connfd);
            } else {
                create_new_thread(slot, gci);
            }
        }
    }


    /* 6. Wait for all threads to finish */

    for (i = 0; i < MAX_THREADS; i++) {
        if (!thread_man[i].available) {
            pthread_join(thread_man[i].thread_id, NULL);
            thread_man[i].available = true;
        }
    }


    /* 7. Clean up server socket */

    close(sockfd);

    return 0;
}


void create_new_thread(int slot, HANDLER_PARAMS_PTR gci) {
    thread_man[slot].exited = false;
    thread_man[slot].available = false;
    inc_thread_count_safe();
    pthread_create(&thread_man[slot].thread_id, NULL, client_handler, gci);
}


HANDLER_PARAMS_PTR get_client_info(int slot, int connfd, struct sockaddr_in client_address) {
    HANDLER_PARAMS_PTR params = malloc(sizeof (struct handler_params));
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

void write_log(char *s) {
    printf("%s\n", s);
}
