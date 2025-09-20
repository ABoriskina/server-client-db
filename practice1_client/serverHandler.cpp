#include "serverHandler.hpp"
#include <iostream>

ServerHandler::~ServerHandler() {
    closeSocket();
}

int ServerHandler::startCommunication(const std::string& data) {
    if (openSocket() != 0) {
        return -1;
    }
    
    if (sendMessage(data) != 0) {
        closeSocket();
        return -1;
    }
    
    int result = waitForServer();
    closeSocket();
    return result;
}

int ServerHandler::waitForServer() {
    valread = read(client_fd, buffer, sizeof(buffer) - 1);
    if (valread > 0) {
        buffer[valread] = '\0';
        return handleMessage(buffer);
    } else {
        syslog(LOG_ERR, "Read from server failed");
    }
}

int ServerHandler::handleMessage(const char* data) {
    std::string response(data);
    if (response == "true") {
        syslog(LOG_INFO, "Authentication successful");
        return DATABASE_USER_FOUND;
    } else {
        syslog(LOG_WARNING, "Authentication failed");
        return DATABASE_USER_NOT_FOUND;
    }
    return 0;
}

int ServerHandler::sendMessage(const std::string& data) {
    ssize_t bytes_sent = send(client_fd, data.c_str(), data.length(), 0);
    if (bytes_sent < 0) {
        syslog(LOG_ERR, "Send failed");
        return -1;
    }
    return 0;
}

int ServerHandler::openSocket() {
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syslog(LOG_ERR, "Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        syslog(LOG_ERR, "Invalid address");
        closeSocket();
        return -1;
    }

    if (connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        syslog(LOG_ERR, "Connection to server failed");
        closeSocket();
        return -1;
    }
    
    isActive = true;
    syslog(LOG_INFO, "Connected to server");
    return 0;
}

void ServerHandler::closeSocket() {
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
    isActive = false;
}