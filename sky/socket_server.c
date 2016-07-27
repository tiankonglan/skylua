#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "sky.h"
#include "socket_server.h"
#include "sky_mq.h"

#define MIN_MALLOC_SIZE 128

struct socket_server{
    int ep_fd;
    int listen_fd;
    struct socket* sock_set[1000];   //hash结构保存clientfd映射
};

struct socket{
    int fd;
    char* buffer;
    int len;
};

static struct  socket_server* ss = NULL;

static void set_nonblock(int fd)
{
    int flag = fcntl(fd, F_GETFL);
    if (flag < 0)
    {
        perror("fcntl GETFL error");
        return;
    }

    flag = flag | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) < 0)
    {
        perror("fcntl SETFL error");
        return;
    }
}

void sock_init()
{
    //Todo: 内存管理考虑 tmalloc
    ss = malloc(sizeof(struct socket_server));
    
    struct sockaddr_in serveraddr;
    ss->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serveraddr, 0, sizeof(struct sockaddr_in));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(3344);

    struct epoll_event ev;
    ss->ep_fd = epoll_create(256);
    ev.data.fd = ss->listen_fd;
    ev.events = EPOLLIN;

    epoll_ctl(ss->ep_fd, EPOLL_CTL_ADD, ss->listen_fd, &ev);
    if ( bind(ss->listen_fd, (struct sockaddr*)&serveraddr, sizeof(struct sockaddr)) != 0)
    {
        printf("socket bind error %d \n", errno);
        exit(1);
    }

    listen(ss->listen_fd, 50);
    set_nonblock(ss->listen_fd);

    int flag = 1; 
    setsockopt (ss->listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
}

int sock_poll_wait()
{
    int ret = 0;
    struct epoll_event events[20];
    int nfds = epoll_wait(ss->ep_fd, events, 20, -1);
    int i ;
    for (i = 0; i < nfds; i++)
    {
        if (events[i].data.fd == ss->listen_fd)
        {   
            struct sockaddr_in clientaddr;
            socklen_t clientlen;
            int client_fd = accept(ss->listen_fd, (struct sockaddr*)&clientaddr, &clientlen);
            //Todo: 加入异常处理

             struct socket* s = malloc(sizeof(struct socket));
             s->fd = client_fd;
            ss->sock_set[client_fd] = s;

            struct epoll_event ev;
            ev.data.fd = client_fd;
            ev.events = EPOLLIN;

            epoll_ctl(ss->ep_fd, EPOLL_CTL_ADD,  client_fd, &ev);
        }
        else if (events[i].events & EPOLLIN) 
        {
            int client_fd = events[i].data.fd;
            char* buffer = malloc(MIN_MALLOC_SIZE);   //目前设定为128
            int recv_num;
           
            //接收到的数据经过msgparser进行处理后，丢到消息队列中
            recv_num = recv(client_fd, buffer, MIN_MALLOC_SIZE , 0);
            printf("echo buffer:  %s \n", buffer);

            if (recv_num == 0) 
            {
                printf("client socket close %d  \n", client_fd);
                close(client_fd);
                continue;
            }

            //hash结构中去查找fd对应的socket结构
            struct socket* s = ss->sock_set[client_fd];
            s->fd = client_fd;
            s->buffer = buffer;
            s->len = recv_num;

            struct message* msg = malloc(sizeof(struct message));
            msg->buffer = s->buffer;    //这里应该是由包处理程序malloc的一个报的大小
            msg->len = s->len;
            msg->next = NULL;
            msg->fd = client_fd;

            //只有在有消息输入的时候， 才触发逻辑线程
            message_queue_push(msg);

            ret = 1;
        }
        else if (events[i].events & EPOLLOUT)
        {
            //Todo: 处理写事件
        }
    }
    return ret;
}

