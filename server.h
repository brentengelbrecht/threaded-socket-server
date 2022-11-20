#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>


/**********************************************************************/
#define PORT 9009       /* Port to listen on for server connections   */
#define MAX_THREADS 3   /* Maximum number of threads to use           */
/**********************************************************************/


struct handler_params {
    int slot;
    int connfd;
    struct sockaddr_in client_address;
};


#define HANDLER_PARAMS_PTR struct handler_params *
#define SOCKADDR_PTR struct sockaddr *


pthread_mutex_t lock;
bool done = false;

int thread_count = 0;
struct thread_management {
    pthread_t thread_id;
    bool exited;
    bool available;
} thread_man[MAX_THREADS];


int get_thread_count_safe();
void inc_thread_count_safe();
void dec_thread_count_safe();
int get_next_slot();
HANDLER_PARAMS_PTR get_client_info(int slot, int connfd, struct sockaddr_in client_address);
void create_new_thread(int next_slot, HANDLER_PARAMS_PTR gci);
void write_log(char *s);
bool initialise();

