#ifndef SERVER_H
#define SERVER_H

#include <lua.hpp>
#include <string>
#include <memory>
#include <asio.hpp>

class Server {
public:
    Server(int port, const std::string& db_host);
    ~Server();
    void start();
    void stop();
    void load_lua_script(const std::string& script_path);
    void execute_lua(const std::string& lua_code);
private:
    int port_;
    std::string db_host_;
    std::unique_ptr<asio::io_context> io_context_;
    lua_State* lua_state_;
    void setup_lua_bindings();
};

#endif
