#include "database.h"
#include <iostream>

Database::Database(const std::string& host, const std::string& user,
                   const std::string& password, const std::string& database)
    : host_(host), user_(user), password_(password), database_(database),
      connection_(nullptr) {
    connection_ = mysql_init(nullptr);
}

Database::~Database() {
    disconnect();
    if (connection_) {
        mysql_close(connection_);
    }
}

bool Database::connect() {
    if (!mysql_real_connect(connection_, host_.c_str(), user_.c_str(),
                           password_.c_str(), database_.c_str(),
                           3306, nullptr, 0)) {
        std::cerr << "Connection failed: " << mysql_error(connection_) << std::endl;
        return false;
    }
    std::cout << "Connected to MariaDB: " << database_ << std::endl;
    return true;
}

void Database::disconnect() {
    if (connection_) {
        mysql_close(connection_);
    }
}

bool Database::is_connected() const {
    return connection_ != nullptr;
}

std::string Database::execute_query(const std::string& query) {
    if (mysql_query(connection_, query.c_str())) {
        return "Query error: " + std::string(mysql_error(connection_));
    }
    MYSQL_RES* result = mysql_store_result(connection_);
    if (!result) {
        return "No result";
    }
    std::string output;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        output += row[0];
        output += ",";
    }
    mysql_free_result(result);
    return output;
}

bool Database::execute_update(const std::string& query) {
    if (mysql_query(connection_, query.c_str())) {
        std::cerr << "Update error: " << mysql_error(connection_) << std::endl;
        return false;
    }
    return true;
}
