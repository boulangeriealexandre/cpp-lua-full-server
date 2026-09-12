#include "server.h"
#include <iostream>
#include <signal.h>

static Server* g_server = nullptr;

void signal_handler(int signal) {
    std::cout << "\nShutting down server..." << std::endl;
    if (g_server) {
        g_server->stop();
    }
    exit(signal);
}

int main(int argc, char* argv[]) {
    try {
        int port = 8080;
        std::string db_host = "localhost";
        
        if (argc > 1) port = std::stoi(argv[1]);
        if (argc > 2) db_host = argv[2];
        
        std::cout << "Starting C++ Lua Server..." << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Database Host: " << db_host << std::endl;
        
        Server server(port, db_host);
        g_server = &server;
        
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        server.load_lua_script("scripts/example.lua");
        server.start();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
