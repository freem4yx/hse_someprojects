#pragma once

#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

#define BUFFERSIZE 256
#define HISTORYSIZE 10

int Socket(int domain, int type, int protocol);

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void Listen(int sockfd, int backlog);

int Accept(int sockfd, sockaddr *addr, socklen_t *addrlen);

void Connect(int sockfd, const sockaddr *addr, socklen_t addrlen);

void Inet_pton(int af, const char *src, void* dst);

void Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout);

long ReadClient(int fd, void* buf, size_t count);

void Close(int fd);
