#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

#include <string>
#include <netinet/in.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include "dbAPI.hpp"
#define PORT 8080

class ClientHandler {
public:
    int startCommunication();
    ~ClientHandler();
private:
    dbAPI* dbapi;
    int server_fd;
    int new_socket;
    ssize_t valread;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    bool isActive = false;
    char buffer[1024];

    void waitForClient();
    int openSocket();
    void closeSocket();
    int handleMessage(const std::string&);
};

#endif