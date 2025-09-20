#include "dbAPI.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

dbAPI::dbAPI() : connection(nullptr) {}

dbAPI::~dbAPI() {
    closeConnection();
}

std::pair<std::string, std::string> parseClientData(const std::string& data) {
    size_t colon_pos = data.find(':');
    if (colon_pos == std::string::npos) {
        return {data, ""};
    }
    
    std::string login = data.substr(0, colon_pos);
    std::string password = data.substr(colon_pos + 1);
    
    return {login, password};
}

int dbAPI::getDataFromDB(std::string data) {
    try {
        auto [login, password] = parseClientData(data);
        syslog(LOG_INFO, "Received login: %s", login.c_str());

        if (!connection || !connection->is_open()) {
            if (setConnection() != DATABASE_CONN_OK) {
                syslog(LOG_ERR, "No database connection");
                return DATABASE_CONN_ERR;
            }
        }

        pqxx::work txn(*connection);
        std::string query = "SELECT * FROM users WHERE login = $1 AND pass = $2";
        pqxx::result result = txn.exec_params(query, login, password);

        if (result.empty()) {
            syslog(LOG_INFO, "No data found for login: %s", login.c_str());
            return DATABASE_USER_NOT_FOUND;
        }

        for (auto row : result) {
            std::cout << "User found: " << row["login"].as<std::string>() 
                      << ", ID: " << row["id"].as<int>() << std::endl;
        }

        txn.commit();
        return DATABASE_USER_FOUND;

    } catch (const std::exception &e) {
        syslog(LOG_ERR, "Database error in getDataFromDB: %s", e.what());
        return DATABASE_CONN_ERR;
    }
}

int dbAPI::setConnection() {
    try {
        closeConnection();
        
        const char* dbHost = std::getenv("DB_HOST");
        const char* dbPort = std::getenv("DB_PORT"); 
        const char* dbName = std::getenv("DB_NAME");
        const char* dbUser = std::getenv("DB_USER");
        const char* dbPass = std::getenv("DB_PASSWORD");

        if (!dbHost || !dbPort || !dbName || !dbUser || !dbPass) {
            syslog(LOG_ERR, "Missing database environment variables");
            return DATABASE_CONN_ERR;
        }

        std::string connectionString = 
            "host=" + std::string(dbHost) + " " +
            "port=" + std::string(dbPort) + " " +
            "dbname=" + std::string(dbName) + " " +
            "user=" + std::string(dbUser) + " " +
            "password=" + std::string(dbPass);

        connection = new pqxx::connection(connectionString);

        if (connection->is_open()) {
            syslog(LOG_INFO, "Connection established");
            return DATABASE_CONN_OK;
        }
        
        return DATABASE_CONN_ERR;
        
    } catch (const std::exception &e) {
        syslog(LOG_ERR, "Error while opening database: %s", e.what());
        return DATABASE_CONN_ERR;
    }
}

int dbAPI::closeConnection() {
    if (connection) {
        connection->close();
        delete connection;
        connection = nullptr;
        syslog(LOG_INFO, "Connection closed");
    }
    return DATABASE_CONN_CLOSED;
}