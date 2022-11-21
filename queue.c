#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"


QUEUE_PTR create_new_queue() {
    QUEUE_PTR p = (QUEUE_PTR)malloc(sizeof(QUEUE));
    if (p == NULL) {
        printf("Memory allocation for queue failed!\n");
        return NULL;
    }
    p->node_count = 0;
    p->tail = NULL;
    p->head = NULL;
}


void destroy_queue(QUEUE_PTR q) {
    QUEUE_NODE_PTR p = q->head;
    while (p != NULL) {
        QUEUE_NODE_PTR r = p;
        p = p->next;
        if (r->data != NULL) {
            free(r->data);
        }
        free(r);
    }
    free(q);
}


void process_queue(QUEUE_PTR q, void (*proc)(void *data)) {
    QUEUE_NODE_PTR p = q->head;
    while (p != NULL) {
        proc(p->data);
        p = p->next;
    }
}


bool enqueue(QUEUE_PTR q, void *data) {
    if (q == NULL) {
        printf("Queue isn\'t allocated!\n");
        return false;
    }
    QUEUE_NODE_PTR n = (QUEUE_NODE_PTR)malloc(sizeof(QUEUE_NODE));
    if (n == NULL) {
        printf("Memory allocation for queue_node failed!\n");
        return false;
    }
    n->data = data;
    n->next = NULL;

    if (q->tail == NULL) {
        q->head = q->tail = n;
    } else {
        q->tail->next = n;
        q->tail = n;
    }
    q->node_count++;
    return true;
}


void *dequeue(QUEUE_PTR q) {
    if (q->head == NULL) {
        return NULL;
    }

    QUEUE_NODE_PTR p = q->head;
    q->node_count--;

    if (q->head != q->tail) {
        q->head = q->head->next;
    } else {
        q->head = NULL;
        q->tail = NULL;
    }

    void *data = p->data;
    free(p);
    return data;
}


int queue_size(QUEUE_PTR q) {
    if (q == NULL) {
        return 0;
    }
    return q->node_count;
}
