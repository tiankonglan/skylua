#ifndef SKYNET_H
#define SKYNET_H

#include <pthread.h>

pthread_mutex_t  mutex;
pthread_cond_t  cont;

typedef int (*sky_cb) ();
struct context;
void set_cb(struct context* ctx, sky_cb cb);



#endif