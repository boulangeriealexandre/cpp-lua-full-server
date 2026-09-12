#ifndef VALIDATION_H
#define VALIDATION_H

#include <string>
#include <regex>
#include <vector>

class Validator {
public:
    static bool validate_email(const std::string& email);
    static bool validate_ip(const std::string& ip);
    static bool validate_port(int port);
    static bool validate_json(const std::string& json);
    static bool validate_sql_safe(const std::string& query);
private:
    static const std::regex EMAIL_REGEX;
    static const std::regex IP_REGEX;
    static const std::regex JSON_REGEX;
};

#endif
