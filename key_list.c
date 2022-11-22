#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include "key_list.h"


KEY_LIST_PTR create_new_key_list(int capacity, pthread_mutex_t *lock) {
    KEY_LIST_PTR p = (KEY_LIST_PTR)malloc(sizeof(KEY_LIST));
    if (p == NULL) {
        printf("Memory allocation for key_list failed!\n");
        return NULL;
    }
    p->key_gen = 1;
    p->capacity = capacity;
    p->size = 0;
    p->head = NULL;
    p->lock = lock;
    return p;
}


void destroy_key_list(KEY_LIST_PTR q) {
    KEY_LIST_NODE_PTR p = q->head;
    while (p != NULL) {
        KEY_LIST_NODE_PTR r = p;
        p = p->next;
        if (r->data != NULL) {
            free(r->data);
        }
        free(r);
    }
    free(q);
}


void process_key_list(KEY_LIST_PTR q, void (*proc)(void *data)) {
    KEY_LIST_NODE_PTR p = q->head;
    while (p != NULL) {
        proc(p->data);
        p = p->next;
    }
}


KEY_LIST_NODE_PTR find_key_list_node(KEY_LIST_PTR q, int key) {
    if (q == NULL) {
        printf("Key_list isn\'t allocated!\n");
        return NULL;
    }
    KEY_LIST_NODE_PTR p = q->head;
    while (p != NULL) {
        if (p->key == key) {
            return p;
        }
        p = p->next;
    }
    return NULL;
}


int add_key_list_node(KEY_LIST_PTR q, void *data) {
    if (q == NULL) {
        printf("Key_list isn\'t allocated!\n");
        return -1;
    }
    KEY_LIST_NODE_PTR n = (KEY_LIST_NODE_PTR)malloc(sizeof(KEY_LIST_NODE));
    if (n == NULL) {
        printf("Memory allocation for key_list_node failed!\n");
        return -2;
    }
    n->key = get_next_key(q);
    n->data = data;
    n->next = NULL;

    if (q->head == NULL) {
        q->head = n;
    } else {
        n->next = q->head;
        q->head = n;
    }
    q->size++;
    return n->key;
}


void *remove_key_list_node(KEY_LIST_PTR q, int key) {
    void *data = NULL;

    if (q->head == NULL) {
        return NULL;
    }

    pthread_mutex_lock(q->lock);
    KEY_LIST_NODE_PTR r = NULL, p = q->head;
    while (p != NULL) {
        if (p->key == key) {
            if (r != NULL) {
                r->next = p->next;
            }
            break;
        }
        r = p;
        p = p->next;
    }

    if (p != NULL) {
        q->size--;
        data = p->data;
        free(p);
    }

    if (q->size == 0) {
        q->head = NULL;
    }
    pthread_mutex_unlock(q->lock);

    return data;
}


int get_key_list_size(KEY_LIST_PTR q) {
    int n = 0;
    if (q == NULL) {
        return 0;
    }
    pthread_mutex_lock(q->lock);
    n = q->size;
    pthread_mutex_unlock(q->lock);
    return n;
}


bool set_key_list_size(KEY_LIST_PTR q, int new_size) {
    if (q == NULL) {
        return false;
    }
    pthread_mutex_lock(q->lock);
    q->size = new_size;
    pthread_mutex_unlock(q->lock);
    return true;
}


bool is_key_list_full(KEY_LIST_PTR q) {
    bool r;
    if (q == NULL) {
        return true;
    }
    pthread_mutex_lock(q->lock);
    r = q->capacity <= q->size;
    pthread_mutex_unlock(q->lock);
    return r;
}


int get_next_key(KEY_LIST_PTR q) {
    int n;
    pthread_mutex_lock(q->lock);
    if (q->key_gen > INT_MAX - 10) {
        q->key_gen = 1;
    }
    n = q->key_gen;
    q->key_gen++;
    pthread_mutex_unlock(q->lock);
    return n;
}
