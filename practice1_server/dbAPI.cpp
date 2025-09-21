#include "dbAPI.hpp"
#include "hash.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>

dbAPI::dbAPI() : connection(nullptr) {}

dbAPI::~dbAPI() {
    closeConnection();
}

std::tuple<std::string, std::string, std::string> parseClientData(const std::string& data) {
    size_t first_colon = data.find(':');
    if (first_colon == std::string::npos) {
        return {"", "", ""};
    }
    
    std::string method = data.substr(0, first_colon);
    std::string remaining = data.substr(first_colon + 1);
    
    size_t second_colon = remaining.find(':');
    if (second_colon == std::string::npos) {
        return {method, remaining, ""};
    }
    
    std::string login = remaining.substr(0, second_colon);
    std::string password = remaining.substr(second_colon + 1);
    
    return {method, login, password};
}

int dbAPI::getDataFromDB(std::string data) {
    try {
        auto [method, login, password] = parseClientData(data);
        if (method == "login") {
            syslog(LOG_INFO, "Received login: %s", login.c_str());

            if (!connection || !connection->is_open()) {
                if (setConnection() != DATABASE_CONN_OK) {
                    syslog(LOG_ERR, "No database connection");
                    return DATABASE_CONN_ERR;
                }
            }
            pqxx::work txn(*connection);
            std::string query = "SELECT salt from users WHERE login = $1";
            pqxx::result result = txn.exec_params(query, login);
            if (result.empty()) {
                syslog(LOG_INFO, "No hash-data found for login: %s", login.c_str());
                return DATABASE_USER_NOT_FOUND;
            }

            std::string u_salt = result[0]["salt"].as<std::string>();
            query = "SELECT name, groupid FROM users WHERE login = $1 AND pass = $2";
            std::string u_password = getPasswordHash(u_salt, password);
            result = txn.exec_params(query, login, u_password);

            if (result.empty()) {
                syslog(LOG_INFO, "No data found for login: %s", login.c_str());
                return DATABASE_USER_NOT_FOUND;
            }

            for (auto row : result) {
                std::cout << "User found: " << row["name"].as<std::string>() 
                            << ", Permission group: " << row["groupid"].as<int>() << std::endl;
            }

            txn.commit();
            return DATABASE_USER_FOUND;
        }
        else if (method == "register") {
            syslog(LOG_INFO, "New user with login: %s", login.c_str());

            if (!connection || !connection->is_open()) {
                if (setConnection() != DATABASE_CONN_OK) {
                    syslog(LOG_ERR, "No database connection");
                    return DATABASE_CONN_ERR;
                }
            }

            pqxx::work txn(*connection);
            std::string query = "INSERT into users (login, pass, salt, name, groupid) VALUES ($1, $2, $3, $4, 2)";
            auto [salt, hash] = setPasswordHash(password);
            pqxx::result result = txn.exec_params(query, login, hash, salt, login);

            txn.commit();
            return DATABASE_USER_FOUND;
        }
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
            syslog(LOG_INFO, "Connection with database established");
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

std::pair <std::string, std::string> dbAPI::setPasswordHash(const std::string& password) {
    std::string salt = generate_salt(16);
    std::string password_hash = hash_password(password, salt);
    return {salt, password_hash};
}

std::string dbAPI::getPasswordHash(const std::string& salt, const std::string& password) {
    std::string password_hash = hash_password(password, salt);
    return password_hash;
}