#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sky.h"
#include "sky_mq.h"

struct message_queue {
    struct message* head;
    struct message* tail;
    int len;
};

static struct message_queue* queue; 

void message_queue_init()
{
    queue = malloc(sizeof(struct message_queue));
    queue->tail = NULL;
    queue->head = queue->tail;
}

void message_queue_push(struct message* msg)
{
    struct message* t  = queue->tail;
    if (t == NULL)
    {
        queue->head = queue->tail = msg; 
    }
    else
    {
        t->next = msg;
        queue->tail = msg;
    }
}

struct message* message_queue_pop()
{
    struct message* t = queue->head;
    if (t == NULL)
    {
        return NULL;
    }

    queue->head = t->next;

    if (queue->head == NULL) 
    {
        queue->tail  = NULL;
    }

    return t;
}

