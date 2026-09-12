#include <asio.hpp>
#include <iostream>
#include <memory>
#include <thread>
#include <string>
#include <regex>

using asio::ip::tcp;

// Utility function to get internal IP
std::string get_internal_ip() {
    try {
        asio::io_context io_context;
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type results = resolver.resolve(asio::ip::host_name(), "");
        
        for (const auto& entry : results) {
            auto address = entry.endpoint().address();
            if (!address.is_loopback() && address.is_v4()) {
                return address.to_string();
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to get internal IP: " << e.what() << std::endl;
    }
    return "127.0.0.1";
}

// Utility function to validate IP address
bool is_valid_ip(const std::string& ip) {
    std::regex ipv4(R"(^(\d{1,3}\.){3}\d{1,3}$)");
    return std::regex_match(ip, ipv4);
}

// Utility function to detect if running in Docker
bool is_running_in_docker() {
    // Check for /.dockerenv file
    std::ifstream docker_check("/.dockerenv");
    if (docker_check.good()) {
        return true;
    }
    
    // Check for cgroup v2
    std::ifstream cgroup_check("/proc/self/cgroup");
    std::string line;
    while (std::getline(cgroup_check, line)) {
        if (line.find("docker") != std::string::npos) {
            return true;
        }
    }
    return false;
}

class Session : public std::enable_shared_from_this<Session> {
public:
    using pointer = std::shared_ptr<Session>;
    
    static pointer create(asio::io_context& io_context) {
        return pointer(new Session(io_context));
    }
    
    tcp::socket& socket() {
        return socket_;
    }
    
    void start() {
        async_read();
    }
    
private:
    Session(asio::io_context& io_context) : socket_(io_context) {}
    
    void async_read() {
        auto self(shared_from_this());
        socket_.async_receive(
            asio::buffer(data_),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    std::cout << "Received from " << socket_.remote_endpoint() 
                              << ": " << std::string(data_, length) << std::endl;
                    async_write(length);
                }
            });
    }
    
    void async_write(std::size_t length) {
        auto self(shared_from_this());
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        socket_.async_send(
            asio::buffer(response),
            [this, self](std::error_code ec, std::size_t) {
                if (!ec) {
                    async_read();
                }
            });
    }
    
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
};

class NetworkServer {
public:
    NetworkServer(asio::io_context& io_context, int port)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        
        // Get server info
        std::string internal_ip = get_internal_ip();
        bool in_docker = is_running_in_docker();
        
        std::cout << "=== Network Server Info ===" << std::endl;
        std::cout << "Internal IP: " << internal_ip << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Running in Docker: " << (in_docker ? "Yes" : "No") << std::endl;
        std::cout << "===========================" << std::endl;
        
        start_accept();
    }
    
private:
    void start_accept() {
        Session::pointer new_session = Session::create(io_context_);
        acceptor_.async_accept(new_session->socket(),
            [this, new_session](std::error_code ec) {
                if (!ec) {
                    std::cout << "New connection from: " << new_session->socket().remote_endpoint() << std::endl;
                    new_session->start();
                }
                start_accept();
            });
    }
    
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};
