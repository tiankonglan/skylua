#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <assert.h>
#include <stdlib.h>

#include "sky.h"
#include "socket_server.h"

static const char *load_config = "\
    local f= io.open(\'luaframe/base.lua\')  \
    local chunk = assert(f:read \'a*\')  \
    f:close()   \
    local func = assert(load(chunk, \'=(load)\', \'t\'))()  \
    return  \
";

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

static void* thread_socket(void *p) 
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
      pthread_cond_singal(&cond); 
      pthread_mutex_unlock(&mutex); 
    }
    return NULL;
}

static void* thread_worker(void *p)
{
    while(1)
    {       
            pthread_mutex_lock(&mutex);
            pthread_cont_wait(&cont);         
            pthread_mutex_unlock(&mutex);
            if (cont.cb == NULL) 
            {
                continue;
            }
            cont.cb(&cont, L, 3);
    }
}

int main() {
      struct context cont;
      struct lua_State *L = luaL_newstate();

      luaL_openlibs(L);
      lua_pushlightuserdata(L, &cont);
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

      pthread_t  sock_thread_t;
      create_thread(&sock_thread_t, thread_socket, NULL);
      

      return 0;
}