#include "dbAPI.hpp"
#include "clientHandler.hpp"
#include <syslog.h>
#include <thread>
#include <iostream>

int main(){
    openlog("practice1_server", LOG_PID | LOG_NDELAY, LOG_LOCAL0);
    syslog(LOG_INFO, "Server starting");
    
    try {
        ClientHandler clientHandler;
        std::thread clientConnection([&]() {
            clientHandler.startCommunication();
        });
        
        std::cout << "Server started. Press Enter to stop..." << std::endl;
        std::cin.get();

        clientConnection.join();
        
    } catch (const std::exception& e) {
        syslog(LOG_ERR, "Server error: %s", e.what());
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    syslog(LOG_INFO, "Authentication server stopped");
    closelog();
    return 0;
}