#include "../include/queue.h"

node_t* head = NULL;
node_t* tail = NULL;

void enqueue(void* client_socket)
{
    node_t *newnode = malloc(sizeof(node_t));
    newnode->client_socket = client_socket;
    newnode->next = NULL;
    if (tail == NULL)
        head = newnode;
    else
        tail->next = newnode;
    tail = newnode;
}

void* dequeue()
{
    if (head == NULL)
        return NULL;
    else {
        void* result = head->client_socket;
        node_t* temp = head;
        head = head->next;
        if (head == NULL)
            tail = NULL;
        free(temp);
        return result;
    }
}
