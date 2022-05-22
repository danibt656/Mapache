#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdlib.h>
#include <stdio.h>

struct node {
    struct node* next;
    void *client_socket;
};
typedef struct node node_t;

void enqueue(void* client_socket);

void* dequeue();

#endif // _QUEUE_H
