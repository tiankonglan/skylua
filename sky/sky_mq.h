#ifndef SKY_MQ_H
#define SKY_MQ_H

struct message_queue;
struct message{
    char* buffer;
    int len;
    struct message* next;
    int fd;
};

void message_queue_init();
void message_queue_push(struct message* msg);
struct message* message_queue_pop();

#endif