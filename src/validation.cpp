#include "validation.h"
#include <algorithm>

const std::regex Validator::EMAIL_REGEX(
    R"(([a-zA-Z0-9+._%-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}))"
);

const std::regex Validator::IP_REGEX(
    R"((\d{1,3}\.){3}\d{1,3})"
);

const std::regex Validator::JSON_REGEX(
    R"(^\{.*\}$|^\[.*\]$)"
);

bool Validator::validate_email(const std::string& email) {
    return std::regex_match(email, EMAIL_REGEX);
}

bool Validator::validate_ip(const std::string& ip) {
    if (!std::regex_match(ip, IP_REGEX)) return false;
    int parts[4] = {0};
    sscanf(ip.c_str(), "%d.%d.%d.%d", &parts[0], &parts[1], &parts[2], &parts[3]);
    for (int i = 0; i < 4; i++) {
        if (parts[i] < 0 || parts[i] > 255) return false;
    }
    return true;
}

bool Validator::validate_port(int port) {
    return port > 0 && port < 65536;
}

bool Validator::validate_json(const std::string& json) {
    return std::regex_match(json, JSON_REGEX);
}

bool Validator::validate_sql_safe(const std::string& query) {
    std::vector<std::string> dangerous = {"DROP", "DELETE", "TRUNCATE"};
    std::string upper_query = query;
    std::transform(upper_query.begin(), upper_query.end(), upper_query.begin(), ::toupper);
    for (const auto& danger : dangerous) {
        if (upper_query.find(danger) != std::string::npos) {
            return false;
        }
    }
    return true;
}
