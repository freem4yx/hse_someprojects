#include "wrappers.h"
#include <cstdio>
#include <cstdlib>
#include <sys/select.h>

int Socket(int domain, int type, int protocol) {
    int res = socket(domain, type, protocol);
    if (res == -1) {
        perror("Socket");
        exit(EXIT_FAILURE);
    }
    return res;
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int res = bind(sockfd, addr, addrlen);
    if (res == -1) {
        perror("Bind");
        exit(EXIT_FAILURE);
    }
}

void Listen(int sockfd, int backlog) {
    int res = listen(sockfd, backlog);
    if (res == -1) {
        perror("Listen");
        exit(EXIT_FAILURE);
    }
}

int Accept(int sockfd, sockaddr *addr, socklen_t *addrlen) {
    int res = accept(sockfd, addr, addrlen);
    if (res == -1) {
        perror("Accept");
        exit(EXIT_FAILURE);
    }
    return res;
}



void Connect(int sockfd, const sockaddr *addr, socklen_t addrlen) {
    int res = connect(sockfd, addr, addrlen);
    if (res == -1) {
        perror("Connect");
        exit(EXIT_FAILURE);
    }
}

void Inet_pton(int af, const char *src, void* dst) {
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

void Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout) {
    if (select(nfds, readfds, writefds, exceptfds, timeout) == -1) {
        perror("Select");
        exit(EXIT_FAILURE);
    }
}

long ReadClient(int fd, void* buf, size_t count) {
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

void Close(int fd) {
    if (close(fd) == -1) {
        perror("Close");
        exit(EXIT_FAILURE);
    }
}
