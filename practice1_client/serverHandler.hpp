#ifndef SERVERHANDLER_HPP
#define SERVERHANDLER_HPP

#include <string>
#include <netinet/in.h>
#include <cstring>
#include <sys/socket.h>
#include <unistd.h>
#include <syslog.h>
#include <arpa/inet.h>

#define PORT 8080

class ServerHandler {
public:
    int startCommunication(const std::string& data);
    ~ServerHandler();
    
private:
    struct sockaddr_in serv_addr;
    char buffer[1024];
    int status;
    int valread;
    int client_fd;
    bool isActive;
    
    void waitForServer();
    int openSocket();
    void closeSocket();
    int sendMessage(const std::string& data);
    int handleMessage(const char* data);
};

#endif