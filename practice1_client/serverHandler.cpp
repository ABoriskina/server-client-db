#include "serverHandler.hpp"
#include <iostream>

ServerHandler::~ServerHandler() {
    if (ctx) {
        SSL_CTX_free(ctx);
        ctx = nullptr;
    }
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
    char message[1024];
    int len = tls_recv(ssl, message, sizeof(message));
    std::string(message, message + len);
    if (len > 0) {
        return handleMessage(message);
    } else {
        syslog(LOG_ERR, "Read from server failed");
    }
    return 1;
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
    ctx = tls_client_ctx("certs/server.crt");
    ssl = tls_wrap_fd_as_client(ctx, client_fd, "localhost");
    tls_send(ssl, data.data(), (int)data.size());
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
    if (ssl){
        tls_close(ssl);
        ssl = nullptr;
    }
    if (client_fd >= 0) {
        close(client_fd);
        client_fd = -1;
    }
    isActive = false;
}