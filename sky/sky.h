#ifndef SKY_H
#define SKY_H

#include <pthread.h>

pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;

typedef int (*sky_cb) ();
struct context;
void set_cb(struct context* ctx, sky_cb cb);

#endif