#ifndef SKY_MQ_H
#define SKY_MQ_H

struct message_queue {
    struct message* head;
    int len;
};

struct message {
    char* buffer;
    int len;
    struct message* next;
};

#endif