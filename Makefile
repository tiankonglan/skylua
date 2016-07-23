CFLAGS = -g -O2 -Wall 
CC = gcc
EXPAND_LIBS = -lpthread -lm -ldl -lrt
SHARED := -fPIC --shared
LIBS = /usr/local/lib/liblua.a 
CORE_SRC = core/
EXPORT =  -Wl,-E

all: luaclib/core.so main

luaclib/core.so:luaclib/lua_core.c
	$(CC) $(CFLAGS) $(SHARED)  $^  -o $@ -I./sky

main: sky/sky_main.c  sky/socket_server.c
	$(CC) $(CFLAGS) -o $@ $^  -I./sky  $(LDFLAGS) $(EXPORT)  $(LIBS)  $(EXPAND_LIBS)

clean:
	rm -rf luaclib/core.so
	rm -rf main