#ifndef ATSC_SERVER_H
#define ATSC_SERVER_H

#include "atsc_config.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "tcp_socket.h"

class ATSC_Server {
public:
    ATSC_Server(int port = 7000, int timeout_ms = 1000);
    void run();

private:
    ATSC_Config config;
    L1_Calculator calculator;
    L1_Generator generator;
    L1_Interpreter interpreter;
    cl_tcp_socket web_server;
    int port;
    int timeout_ms;

    void handle_connection();
    std::string process_received_data(const std::string& data);
};

#endif
