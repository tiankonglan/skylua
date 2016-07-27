#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

struct socket_server; 
struct socket;

void sock_init();
int sock_poll_wait();

#endif