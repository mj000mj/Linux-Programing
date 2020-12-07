#ifndef __WRAP_H_
#define __WRAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <ctype.h>
#include <sys/types.h>

void Perr_exit(char *err);

int Socket(int domain, int type, int protocol);
int Bind(int sockfd, struct  sockaddr  *my_addr,  socklen_t addrlen);
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int Accept(int   s,   struct  sockaddr  *addr,  socklen_t *addrlen);
int Listen(int s, int backlog);
int Close(int fd);

ssize_t Read(int fd, void *ptr, size_t nbytes);
ssize_t Write(int fd, const void *ptr, size_t nbytes);

ssize_t Readn(int fd, void *vptr, size_t n);
ssize_t Writen(int fd, const void *vptr, size_t n);
ssize_t my_read(int fd, char *ptr);
ssize_t Readline(int fd, void *vptr, size_t maxlen);

#endif