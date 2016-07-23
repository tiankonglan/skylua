struct message_queue {
    struct message* head;
    struct message* tail;
    int len;
};

struct message {
    char* buffer;
    int len;
    struct message* next;
};

static message_queue* queue = Malloc(sizeof(struct message_queue));

void message_queue_init(struct message_queue* queue)
{
    queue->tail = NULL;
    queue->head = queue->tail;
}

void message_queue_push(struct message_queue* queue, struct message* msg)
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

void message_queue_pop(struct message_queue* queue, struct message* msg)
{
    struct message* t = queue->head;
    if (t == NULL)
    {
        printf("queue is empty \n");
        return NULL;
    }

    queue->head = t->next;
    return t;
}

