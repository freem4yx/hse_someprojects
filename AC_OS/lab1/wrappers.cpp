#include "wrappers.h"
#include <cstdio>
#include <cstdlib>
#include <sys/select.h>

int socket_wrapped(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    return res;
}

void bind_wrapped(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("Bind");
        exit(EXIT_FAILURE);
    }
}

void listen_wrapped(int sockfd, int backlog) {
    int res = listen(sockfd, backlog);
    if (res == -1) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
}

int accept_wrapped(int sockfd, sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, addr, addrlen);
    if (res == -1) {
        perror("Accept");
        exit(EXIT_FAILURE);
    }
    return res;
}



void connect_wrapped(int sockfd, const sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res == -1) {
        perror("Connect");
        exit(EXIT_FAILURE);
    }
}

void inet_pton_wrapped(int af, const char *src, void* dst) {
    int res = inet_pton(af, src, dst);
    if (res == 0) {
        std::cout << "connection is valid, but source is empty\n";
        exit(EXIT_FAILURE);
    }
    if (res == -1) {
        perror("Inet_pton");
        exit(EXIT_FAILURE);
    }
}

void select_wrapped(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout) {
    if (select(nfds, readfds, writefds, exceptfds, timeout) == -1) {
        perror("Select");
        exit(EXIT_FAILURE);
    }
}

long readClient_wrapped(int fd, void* buf, size_t count) {
    long bytesReaded = read(fd, buf, count);
    if (bytesReaded == -1) {
        perror("Read");
        exit(EXIT_FAILURE);
    }
    if (bytesReaded == 0) {
        std::cout << "Server closed\n";
        exit(0);
    }
    return bytesReaded;
}

void close_wrapped(int fd) {
    if (close(fd) == -1) {
        perror("Close");
        exit(EXIT_FAILURE);
    }
}
