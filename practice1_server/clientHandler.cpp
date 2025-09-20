#include "clientHandler.hpp"

ClientHandler::~ClientHandler() {
    delete dbapi;
}

int ClientHandler::startCommunication(){
    dbapi = new dbAPI();
    if (openSocket() != 0) {
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
        
        close(new_socket);
        syslog(LOG_INFO, "Client disconnected, waiting for new connection");
    }
}

void ClientHandler::handleClient() {
    while (true) {
        valread = read(new_socket, buffer, 1024 - 1);
        if (valread > 0) {
            buffer[valread] = '\0';
            handleMessage(std::string(buffer));
        } else if (valread == 0) {
            syslog(LOG_INFO, "Client disconnected");
            break;
        } else {
            syslog(LOG_ERR, "Read error");
            break;
        }
        sleep(2);
    }
}

int ClientHandler::handleMessage(const std::string& data) {
    int result = dbapi->getDataFromDB(data);
    
    const char* response;
    if (result == DATABASE_USER_FOUND) {
        response = "true";
    } else {
        response = "false";
    }
    send(new_socket, response, strlen(response), 0);
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
    if (new_socket >= 0) close(new_socket);
    if (server_fd >= 0) close(server_fd);
    isActive = false;
    syslog(LOG_INFO, "Socket closed");
}