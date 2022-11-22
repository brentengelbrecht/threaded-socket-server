#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>


/**********************************************************************/
#define PORT 9014       /* Port to listen on for server connections   */
#define MAX_THREADS 2   /* Maximum number of threads to use           */
#define SLEEP 8         /* Maximum time to put client thread to sleep */
/**********************************************************************/


struct handler_params {
    int thread_key;
    int connfd;
    struct sockaddr_in client_address;
    time_t start;
};


#define HANDLER_PARAMS struct handler_params
#define HANDLER_PARAMS_PTR struct handler_params *
#define SOCKADDR_PTR struct sockaddr *


HANDLER_PARAMS_PTR get_client_info(int connfd, struct sockaddr_in client_address);
bool initialise();
bool cleanup_socket_connections(void *p);
bool cleanup_threads(void *p);
void signal_handler(int);
