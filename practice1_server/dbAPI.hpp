#ifndef DBAPI_HPP
#define DBAPI_HPP

#include <syslog.h>
#include <pqxx/pqxx>

#define DATABASE_USER_NOT_FOUND 4
#define DATABASE_USER_OK 3
#define DATABASE_USER_FOUND 3
#define DATABASE_CONN_CLOSED 2
#define DATABASE_CONN_ERR 1
#define DATABASE_CONN_OK 0

class dbAPI {
public:
    dbAPI();
    ~dbAPI();
    int getDataFromDB(std::string data);
private:
    int setConnection();
    int closeConnection();
    std::pair <std::string, std::string> setPasswordHash(const std::string& password);
    std::string getPasswordHash(const std::string& salt, const std::string& password);
    pqxx::connection* connection;
};

#endif