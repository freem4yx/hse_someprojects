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

int socket_wrapped(int domain, int type, int protocol);

void bind_wrapped(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void listen_wrapped(int sockfd, int backlog);

int accept_wrapped(int sockfd, sockaddr *addr, socklen_t *addrlen);

void connect_wrapped(int sockfd, const sockaddr *addr, socklen_t addrlen);

void inet_pton_wrapped(int af, const char *src, void* dst);

void select_wrapped(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout);

long readClient_wrapped(int fd, void* buf, size_t count);

void close_wrapped(int fd);
