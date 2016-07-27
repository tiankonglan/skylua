#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>

#include <assert.h>
#include <stdlib.h>

#include "sky.h"
#include "socket_server.h"
#include "sky_mq.h"

static const char *load_config = "\
    local f= io.open(\'luaframe/base.lua\')  \
    local chunk = assert(f:read \'a*\')  \
    f:close()   \
    local func = assert(load(chunk, \'=(load)\', \'t\'))()  \
    return  \
";

struct lua_State *L; 
struct context* ctx;

struct context 
{
    sky_cb cb;
};

void set_cb(struct context* ctx, sky_cb cb)
{
    ctx->cb = cb;
}

static void
create_thread(pthread_t *thread, void *(*start_routine) (void *), void *arg) 
{
  if (pthread_create(thread,NULL, start_routine, arg)) {
    fprintf(stderr, "Create thread failed");
    exit(1);
  }
}

static void* socket_thread(void *p) 
{   
    sock_init();
    for(;;)
    {
      // epoll_wait触发
      int ret = sock_poll_wait();
      if (ret == 0)
      {
          sleep(1);
          continue;
      }
      
      // notify to worker thread 
      pthread_mutex_lock(&mutex);
      pthread_cond_signal(&cont); 
      pthread_mutex_unlock(&mutex); 
    }
    return NULL;
}

static void* worker_thread(void *p)
{
    while(1)
    {       
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&cont, &mutex);         
            pthread_mutex_unlock(&mutex);
            if (ctx->cb == NULL) 
            {
                continue;
            }

            struct message* msg = message_queue_pop();
            if (msg != NULL)
            {
                 ctx->cb(ctx, L, 3);
                 send(msg->fd, msg->buffer, msg->len, 0);
            }
    }
    return NULL;
}

int main() {
      L = luaL_newstate();
      ctx = malloc(sizeof(struct context));

      luaL_openlibs(L);
      lua_pushlightuserdata(L, ctx);
      lua_setfield(L, LUA_REGISTRYINDEX, "context");

      int err = luaL_loadstring(L, load_config);
      assert(err == LUA_OK);

      printf("prepare call lua function \n");
      err = lua_pcall(L, 0, 0, 0);
      printf("after call lua function %d \n", err);
      if (err)
      {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            lua_close(L);
            return 1;
      }

      pthread_mutex_init(&mutex, NULL);
      pthread_cond_init(&cont, NULL);

      message_queue_init();

      pthread_t  sock_thread_t;
      create_thread(&sock_thread_t, socket_thread, NULL);
    
      pthread_t worker_thread_t;
      create_thread(&worker_thread_t, worker_thread, NULL);

      pthread_join(sock_thread_t, NULL);
      pthread_join(worker_thread_t, NULL);

      return 0;
}