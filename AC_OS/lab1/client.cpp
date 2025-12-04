#include "wrappers.h"
#include <cstdio>
#include <ostream>
#include <string>
#include <sys/select.h>
#include <unistd.h>

int main() {
    int fd = socket_wrapped(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {0, 0, 0, 0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(32000);
    inet_pton_wrapped(AF_INET, "127.0.0.1", &addr.sin_addr);
    connect_wrapped(fd, (sockaddr*) &addr, sizeof(addr));
    std::cout << "Enter your name:\n";
    std::string name;
    std::getline(std::cin, name);

    std::string message;
    long bytesReaded;
    char buffer[BUFFERSIZE];

    int maxSocket = fd > STDIN_FILENO ? fd : STDIN_FILENO;

    std::cout << "Connected to server. Commands: ':history' - show history, ':exit' - quit\n";

    while(1) {

        fd_set inputs;
        FD_ZERO(&inputs);

        FD_SET(STDIN_FILENO, &inputs);
        FD_SET(fd, &inputs);

        select_wrapped(maxSocket + 1, &inputs, NULL, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &inputs)) {
            if (!std::getline(std::cin, message)) break;

            if (message == ":exit") break;

            write(STDOUT_FILENO, "\r\033[1A\033[K", 8);

            if (message == ":history") {
                write(fd, message.c_str(), message.size());
                continue;
            }

            message = name + ": " + message;
            write(fd, message.c_str(), message.size());

        }

        if (FD_ISSET(fd, &inputs)) {
            bytesReaded = readClient_wrapped(fd, buffer, BUFFERSIZE - 1);

            write(STDOUT_FILENO, buffer, bytesReaded);
        }
    }

    std::cout << "Disconnected from server\n";
    close(fd);
    return 0;
}
