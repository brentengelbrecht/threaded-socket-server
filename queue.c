#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "queue.h"


struct queue *create_new_queue() {
    struct queue *p = (struct queue *)malloc(sizeof(struct queue));
    if (p == NULL) {
        printf("Memory allocation for queue failed!\n");
        return NULL;
    }
    p->node_count = 0;
    p->tail = NULL;
    p->head = NULL;
}


void destroy_queue(struct queue *q) {
    struct queue_node *p = q->head;
    while (p != NULL) {
        struct queue_node *r = p;
        p = p->next;
        if (r->data != NULL) {
            free(r->data);
        }
        free(r);
    }
    free(q);
}


void process_queue(struct queue *q, void (*proc)(void *data)) {
    struct queue_node *p = q->head;
    while (p != NULL) {
        proc(p->data);
        p = p->next;
    }
}


bool enqueue(struct queue *q, void *data) {
    if (q == NULL) {
        printf("Queue isn\'t allocated!\n");
        return false;
    }
    struct queue_node *n = (struct queue_node *)malloc(sizeof(struct queue_node));
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


void *dequeue(struct queue *q) {
    if (q->head == NULL) {
        return NULL;
    }

    struct queue_node *p = q->head;
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


int queue_size(struct queue *q) {
    if (q == NULL) {
        return 0;
    }
    return q->node_count;
}
