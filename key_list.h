#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct key_list_node KEY_LIST_NODE;
typedef struct key_list_node * KEY_LIST_NODE_PTR;

typedef struct key_list KEY_LIST;
typedef struct key_list * KEY_LIST_PTR;

struct key_list_node {
    int key;
    KEY_LIST_NODE_PTR next;
    void *data;
};

struct key_list {
    int key_gen, capacity, size;
    KEY_LIST_NODE_PTR head;
    pthread_mutex_t *lock;
};


KEY_LIST_PTR create_new_key_list(int capacity, pthread_mutex_t *lock);
void destroy_key_list(KEY_LIST_PTR q);
void process_key_list(KEY_LIST_PTR q, bool (*proc)(void *data));
KEY_LIST_NODE_PTR find_key_list_node(KEY_LIST_PTR q, int key);
int add_key_list_node(KEY_LIST_PTR q, void *data);
void *remove_key_list_node(KEY_LIST_PTR q, int key);
int get_key_list_size(KEY_LIST_PTR q);
bool set_key_list_size(KEY_LIST_PTR q, int new_size);
bool is_key_list_full(KEY_LIST_PTR q);
int get_next_key(KEY_LIST_PTR q);
