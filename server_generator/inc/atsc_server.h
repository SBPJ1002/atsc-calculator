#ifndef ATSC_SERVER_H
#define ATSC_SERVER_H

#include "atsc_config.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "json_handler.h"
#include <string>
#include <mutex>

class ATSC_Server {
public:
    ATSC_Server(int port = 6000, int timeout_ms = -1);
    void run();

private:
    ATSC_Config config;
    L1_Calculator calculator;
    L1_Generator generator;
    L1_Interpreter interpreter;
    JsonHandler json_handler;
    int port;
    int timeout_ms;
    std::mutex config_mutex;

    void handle_client(int client_fd);
    std::string read_until_newline(int fd);
};

#endif
