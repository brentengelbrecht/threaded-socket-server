#include <stdlib.h>
#include <stdbool.h>


struct queue_node {
    struct queue_node *next;
    void *data;
};

struct queue {
    int node_count;
    struct queue_node *head, *tail;
};


struct queue *create_new_queue();
void destroy_queue(struct queue *q);
void process_queue(struct queue *q, void (*proc)(void *data));
bool enqueue(struct queue *q, void *data);
void *dequeue(struct queue *q);
int queue_size(struct queue *q);
