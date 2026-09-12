#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>
#include <string>
#include <memory>

class Database {
public:
    Database(const std::string& host, const std::string& user,
             const std::string& password, const std::string& database);
    ~Database();
    bool connect();
    void disconnect();
    bool is_connected() const;
    std::string execute_query(const std::string& query);
    bool execute_update(const std::string& query);
private:
    MYSQL* connection_;
    std::string host_;
    std::string user_;
    std::string password_;
    std::string database_;
};

#endif
