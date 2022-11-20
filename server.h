#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>


/**********************************************************************/
#define PORT 9011       /* Port to listen on for server connections   */
#define MAX_THREADS 2   /* Maximum number of threads to use           */
#define SLEEP 60        /* Maximum time to put client thread to sleep */
/**********************************************************************/


struct handler_params {
    int slot;
    int connfd;
    struct sockaddr_in client_address;
};


#define HANDLER_PARAMS_PTR struct handler_params *
#define SOCKADDR_PTR struct sockaddr *


int get_thread_count_safe();
void inc_thread_count_safe();
void dec_thread_count_safe();
int get_next_slot();
HANDLER_PARAMS_PTR get_client_info(int slot, int connfd, struct sockaddr_in client_address);
void create_new_thread(HANDLER_PARAMS_PTR gci);
void write_log(char *s);
bool initialise();
void cleanup_socket_connections(void *p);

