#include <stdio.h>
#include <assert.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "sky.h"

#define True 1
#define False 0

int cb_index = 321;

static int traceback (lua_State *L) 
{
    const char *msg = lua_tostring(L, 1);
    if (msg)
        luaL_traceback(L, L, msg, 1);
    else {
        lua_pushliteral(L, "(no error message)");
    }
    return 1;
}

static int _cb(struct context* ctx, void* ud, int type)
{
    lua_State* L = ud;
    int trace = 1;
    int top = lua_gettop(L);
    if (top == 0)
    {
        lua_pushcfunction(L, traceback);
        lua_rawgetp(L, LUA_REGISTRYINDEX, _cb);
    } else 
    {
        assert(top == 2);
    }

    lua_pushvalue(L, 2);
    
    lua_pushinteger(L, type);

    int r;
    r = lua_pcall(L, 1, 0, trace);

    top = lua_gettop(L);

    if (r == LUA_OK)
    {
        return 0;
    }

    lua_pop(L, 1);
    return 0;
}

static int lcallback(lua_State *L) {
    struct context* ctx = lua_touserdata(L, lua_upvalueindex(1));
    luaL_checktype(L, 1, LUA_TFUNCTION);
    lua_settop(L, 1);
    lua_rawsetp(L, LUA_REGISTRYINDEX,  &_cb);
    set_cb(ctx, _cb);

    return 0;
}

int luaopen_core(lua_State *L) {
    luaL_checkversion(L);

    luaL_Reg l[] = {
        { "callback", lcallback },
        { NULL, NULL },
    };

    //luaL_newlib(L,l);
    luaL_newlibtable(L, l);

    lua_getfield(L, LUA_REGISTRYINDEX, "context");
    struct context* ctx = lua_touserdata(L, -1);
    if (ctx == NULL) {
         return luaL_error(L, "Init context first");
    }

    luaL_setfuncs(L, l, 1);
    return 1;
}










