#include "clientHandler.hpp"

ClientHandler::~ClientHandler() {
    delete dbapi;
    if (ctx) 
        SSL_CTX_free(ctx);
}

int ClientHandler::startCommunication(){
    dbapi = new dbAPI();
    ctx = tls_server_ctx("certs/server.crt", "certs/server.key");
    if (openSocket() != 0) {
        SSL_CTX_free(ctx);
        delete dbapi;
        return 1;
    }
    
    waitForClient();
    closeSocket();
    delete dbapi;
    return 0;
}

void ClientHandler::waitForClient() {
    while (isActive){
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            syslog(LOG_ERR, "Accept failed");
            sleep(2);
            continue;
        }
        syslog(LOG_INFO, "Client connected");
        handleClient();
        syslog(LOG_INFO, "Client disconnected, waiting for new connection");
        usleep(10000);
    }
}

void ClientHandler::handleClient() {
    try {
        ssl = tls_wrap_fd_as_server(ctx, new_socket);
        if (!ssl) {
            syslog(LOG_ERR, "TLS wrap failed");
            close(new_socket);
            return;
        }

        while (true) {
            char socketBuffer[1024];
            int len = tls_recv(ssl, socketBuffer, sizeof(socketBuffer));
            
            if (len == 0) {
                syslog(LOG_INFO, "Client closed connection gracefully");
                break;
            } else if (len < 0) {
                int ssl_error = SSL_get_error(ssl, len);
                syslog(LOG_INFO, "TLS error: %d", ssl_error);
                break;
            } else {
                std::string message(socketBuffer, len);
                handleMessage(message);
            }
        }

    } catch(const std::exception& e){
        syslog(LOG_ERR, "TLS exception: %s", e.what());
    }
}

int ClientHandler::handleMessage(const std::string& data) {
    int result = dbapi->getDataFromDB(data);
    
    std::string response;
    if (result == DATABASE_USER_FOUND) {
        response = "true";
    } else {
        response = "false";
    }
    tls_send(ssl, response.data(), (int)response.size());
    return 0;
}

int ClientHandler::openSocket(){
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        syslog(LOG_ERR, "Socket failed");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        syslog(LOG_ERR, "Setsockopt failed");
        return 1;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "Bind failed");
        return 1;
    }
    if (listen(server_fd, 3) < 0) {
        syslog(LOG_ERR, "Listen failed");
        return 1;
    }

    syslog(LOG_INFO, "Server listening on port %d", PORT);
    isActive = true;
    return 0;
}

void ClientHandler::closeSocket(){
    tls_close(ssl);
    if (new_socket >= 0) 
        close(new_socket);
    if (server_fd >= 0) 
        close(server_fd);
    isActive = false;
    syslog(LOG_INFO, "Socket closed");
}