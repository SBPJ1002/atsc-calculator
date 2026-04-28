#include "atsc_server.h"
#include <iostream>
#include <unistd.h>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cstring>

ATSC_Server::ATSC_Server(int port, int timeout_ms)
    : config(),
      calculator(config),
      generator(config, calculator),
      interpreter(config),
      json_handler(config, calculator, generator, interpreter),
      port(port),
      timeout_ms(timeout_ms)
{
}

void ATSC_Server::run() {
    std::cout << "ATSC 3.0 Server (JSON API)" << std::endl;
    std::cout << "Starting server on port " << port << "..." << std::endl;

    config.load();

    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "ERROR: Failed to create socket" << std::endl;
        exit(-1);
    }

    // Set socket options
    int enable = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(int));

    // Bind
    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "ERROR: Failed to bind socket" << std::endl;
        close(server_fd);
        exit(-1);
    }

    // Listen
    if (listen(server_fd, 10) < 0) {
        std::cerr << "ERROR: Failed to listen" << std::endl;
        close(server_fd);
        exit(-1);
    }

    std::cout << "TCP server initialized successfully!" << std::endl;
    std::cout << "Configuration file: " << config.config_file << std::endl;
    std::cout << "Awaiting connections on port " << port << "..." << std::endl;

    // Accept loop - spawn thread per connection
    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "WARNING: Failed to accept connection" << std::endl;
            continue;
        }

        std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << std::endl;

        // Set read timeout on client socket
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        // Spawn thread to handle client
        std::thread(&ATSC_Server::handle_client, this, client_fd).detach();
    }
}

std::string ATSC_Server::read_until_newline(int fd) {
    std::string buffer;
    char c;

    while (true) {
        int n = recv(fd, &c, 1, 0);
        if (n <= 0) {
            break;
        }
        if (c == '\n') {
            break;
        }
        buffer += c;
    }

    return buffer;
}

void ATSC_Server::handle_client(int client_fd) {
    std::string data = read_until_newline(client_fd);

    if (data.empty()) {
        close(client_fd);
        return;
    }

    std::cout << "Received (" << data.size() << " bytes)" << std::endl;

    // Process JSON request with mutex protection
    std::string reply;
    {
        std::lock_guard<std::mutex> lock(config_mutex);
        reply = json_handler.processRequest(data);
    }

    // Send response
    if (!reply.empty()) {
        std::cout << "Sending response (" << reply.size() << " bytes)" << std::endl;
        size_t total_sent = 0;
        while (total_sent < reply.size()) {
            int n = send(client_fd, reply.c_str() + total_sent, reply.size() - total_sent, 0);
            if (n <= 0) break;
            total_sent += n;
        }
    }

    close(client_fd);
    std::cout << "Connection closed" << std::endl;
}
