#include "wrappers.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/select.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <ctime>

std::vector<int> connected;

class History {
    size_t rowAmount;
    std::vector<std::string> history;
    std::fstream fileHistory;

public:
    History() : rowAmount(0) {
        fileHistory.open("history.txt", std::ios::in | std::ios::out | std::ios::app);
        if (!fileHistory.is_open()) {
            std::ofstream mkfile("history.txt");
            mkfile.close();
            fileHistory.open("history.txt", std::ios::in | std::ios::out | std::ios::app);
        }

        std::string temp;
        while(std::getline(fileHistory, temp)) {
            if (!temp.empty()) {
                ++rowAmount;
                history.push_back(temp);
            }
        }
        fileHistory.clear();
        std::cout << "Loaded " << history.size() << " messages from history\n";
    }

    ~History() {
        save();
        fileHistory.close();
    }

    void save() {
        fileHistory.close();
        fileHistory.open("history.txt", std::ios::out | std::ios::trunc);

        for (size_t i = 0; i < history.size(); ++i) {
            fileHistory << history[i];
            if (i != history.size() - 1) {
                fileHistory << "\n";
            }
        }
        fileHistory.flush();
        std::cout << "History saved: " << history.size() << " messages\n";
    }

    void add(std::string&& message) {

        std::string clean_message = message;
        clean_message.erase(std::remove(clean_message.begin(), clean_message.end(), '\n'), clean_message.end());
        clean_message.erase(std::remove(clean_message.begin(), clean_message.end(), '\r'), clean_message.end());

        history.push_back(std::move(clean_message));
    }

    void add(std::string& message) {

        std::string clean_message = message;
        clean_message.erase(std::remove(clean_message.begin(), clean_message.end(), '\n'), clean_message.end());
        clean_message.erase(std::remove(clean_message.begin(), clean_message.end(), '\r'), clean_message.end());

        history.push_back(clean_message);
    }

    void clear() {
        history.clear();
    }

    bool empty() { return history.empty(); }

    std::string at(size_t pos) const {
        if (pos < history.size()) {
            return history.at(pos);
        }
        return "";
    }

    size_t size() { return history.size(); }
};

void writeToAll(int sender, const char* buffer, size_t size, History* history) {

    std::string message = std::string((char*)buffer, size);
    message.erase(std::remove(message.begin(), message.end(), '\n'), message.end());
    message.erase(std::remove(message.begin(), message.end(), '\r'), message.end());

    if (message.empty()) return;

    std::string message_with_newline = message + "\n";

    for (int elem : connected) {
        if (sender != elem) {
            write(elem, message_with_newline.c_str(), message_with_newline.size());
        }
    }

    // Выводим на сервер
    write(STDOUT_FILENO, message_with_newline.c_str(), message_with_newline.size());

    history->add(message);
}

int main() {
    int server = Socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr = {0, 0, 0, 0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(32000);
    unsigned int addrlen = sizeof(addr);
    Bind(server, (sockaddr *) (&addr), addrlen);
    Listen(server, 5);

    char buffer[BUFFERSIZE];
    History* history = new History;
    std::tm* local_time;

    std::cout << "Server started. History contains " << history->size() << " messages\n";
    std::cout << "'save history' - to save history\n";
    std::cout << "'close server' - to close server\n";
    std::cout << "'clear history' - to clear history\n";

    while (1) {
        fd_set inputs;
        FD_ZERO(&inputs);
        FD_SET(STDIN_FILENO, &inputs);
        FD_SET(server, &inputs);
        int maxSocket = server;

        for (int elem : connected) {
            FD_SET(elem, &inputs);
            if (maxSocket < elem) maxSocket = elem;
        }
        Select(maxSocket + 1, &inputs, NULL, NULL, NULL);

        if (FD_ISSET(server, &inputs)) {
            int newSocket = Accept(server, (sockaddr *) (&addr), &addrlen);
            connected.push_back(newSocket);
            std::cout << "New client connected: " << newSocket << '\n';
        }

        if (FD_ISSET(STDIN_FILENO, &inputs)) {
            char bufServer[BUFFERSIZE];
            long bytes = read(STDIN_FILENO, buffer, BUFFERSIZE);
            if (bytes > 0) {
                std::string consoleInput = std::string(buffer, bytes);
                if (consoleInput.find("save history") != std::string::npos) {
                    history->save();
                    std::cout << "History saved manually\n";
                } else if (consoleInput.find("close server") != std::string::npos) {
                    std::cout << "Closing server...\n";
                    break;
                } else if (consoleInput.find("clear history") != std::string::npos) {
                    history->clear();
                    std::cout << "History cleared\n";
                }
            }
        }

        std::vector<int> copyConnected = connected;

        for (int elem : copyConnected) {
            if (FD_ISSET(elem, &inputs)) {
                long bytesReaded = read(elem, buffer, BUFFERSIZE);
                if (bytesReaded > 0) {
                    std::string output = std::string(buffer, bytesReaded);
                    bytesReaded += 17;

                    char timebuffer[80];
                    std::time_t now = time(nullptr);
                    local_time = std::localtime(&now);
                    std::strftime(timebuffer, sizeof(timebuffer), "[%d-%m-%y %H:%M]", local_time);

                    output = std::string(timebuffer, 16) + " " + output;

                    output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
                    output.erase(std::remove(output.begin(), output.end(), '\r'), output.end());

                    if (output.find(":history") != std::string::npos) {
                        std::cout << "Sending history to client " << elem << "\n";
                        size_t history_size = history->size();
                        size_t send_count = (HISTORYSIZE > history_size) ? history_size : HISTORYSIZE;
                        std::string end_marker;
                        if (send_count > 0) {
                            std::string start_marker = "--- Start of history (" + std::to_string(send_count) + " messages) ---\n";
                            write(elem, start_marker.c_str(), start_marker.size());
                            for (size_t i = history_size - send_count; i < history_size; ++i) {
                                std::string history_line = history->at(i) + "\n";
                                write(elem, history_line.c_str(), history_line.size());
                            }
                            end_marker = "--- End of history (" + std::to_string(send_count) + " messages) ---\n";

                        } else {
                            end_marker = "History is empty\n";
                        }
                        write(elem, end_marker.c_str(), end_marker.size());


                    } else {
                        writeToAll(-1, output.c_str(), bytesReaded, history);
                    }

                } else {
                    if (bytesReaded == 0) {
                        std::cerr << "Client " << elem << " disconnected\n";
                    } else {
                        perror("Read Server");
                    }
                    Close(elem);
                    connected.erase(std::remove(connected.begin(), connected.end(), elem), connected.end());
                }
            }
        }
    }

    // Закрываем все соединения
    for (int client : connected) {
        close(client);
    }
    close(server);

    delete history;
    return 0;
}
