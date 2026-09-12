#include "server.h"
#include <iostream>
#include <lua.hpp>

Server::Server(int port, const std::string& db_host)
    : port_(port), db_host_(db_host), lua_state_(nullptr) {
    lua_state_ = luaL_newstate();
    luaL_openlibs(lua_state_);
    setup_lua_bindings();
}

Server::~Server() {
    if (lua_state_) {
        lua_close(lua_state_);
    }
}

void Server::start() {
    std::cout << "Server started on port " << port_ << std::endl;
}

void Server::stop() {
    std::cout << "Server stopping..." << std::endl;
}

void Server::load_lua_script(const std::string& script_path) {
    if (luaL_dofile(lua_state_, script_path.c_str()) != LUA_OK) {
        std::cerr << "Failed to load Lua script: " << lua_tostring(lua_state_, -1) << std::endl;
    } else {
        std::cout << "Lua script loaded: " << script_path << std::endl;
    }
}

void Server::execute_lua(const std::string& lua_code) {
    if (luaL_dostring(lua_state_, lua_code.c_str()) != LUA_OK) {
        std::cerr << "Lua error: " << lua_tostring(lua_state_, -1) << std::endl;
    }
}

void Server::setup_lua_bindings() {
    lua_pushcfunction(lua_state_, [](lua_State* L) -> int {
        const char* msg = luaL_checkstring(L, 1);
        std::cout << "[Lua] " << msg << std::endl;
        return 0;
    });
    lua_setglobal(lua_state_, "print_from_lua");
}
