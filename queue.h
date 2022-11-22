#include <stdlib.h>
#include <stdbool.h>

typedef struct queue_node QUEUE_NODE;
typedef struct queue_node * QUEUE_NODE_PTR;

typedef struct queue QUEUE;
typedef struct queue * QUEUE_PTR;

struct queue_node {
    QUEUE_NODE_PTR next;
    void *data;
};

struct queue {
    int node_count;
    QUEUE_NODE_PTR head;
    QUEUE_NODE_PTR tail;
};


QUEUE_PTR create_new_queue();
void destroy_queue(QUEUE_PTR q);
void process_queue(QUEUE_PTR q, bool (*proc)(void *data));
bool enqueue(QUEUE_PTR q, void *data);
void *dequeue(QUEUE_PTR q);
int queue_size(QUEUE_PTR q);
