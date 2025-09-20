#ifndef DBAPI_HPP
#define DBAPI_HPP

#include <syslog.h>
#include <pqxx/pqxx>

#define DATABASE_USER_NOT_FOUND 4
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
    pqxx::connection* connection;
};

#endif