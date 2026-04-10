#include "atsc_server.h"
#include <iostream>
#include <unistd.h>

ATSC_Server::ATSC_Server(int port, int timeout_ms)
    : config(),
      calculator(config),
      generator(config, calculator),
      interpreter(config),
      port(port),
      timeout_ms(timeout_ms)
{
}

void ATSC_Server::run() {
    std::cout << "ATSC 3.0 Server" << std::endl;
    std::cout << "Starting server on port " << port << "..." << std::endl;

    config.load();

    web_server.port = port;
    web_server.timeout_ms = timeout_ms;

    if (web_server.init() != SUCCESS) {
        std::cout << "ERROR: Failed to initialize TCP sockets. Exiting..." << std::endl;
        exit(-1);
    }

    std::cout << "TCP server initialized successfully!" << std::endl;
    std::cout << "Configuration file: " << config.config_file << std::endl;
    std::cout << "Awaiting connections..." << std::endl;

    while (1) {
        if (web_server.get_status() == TCP_STATUS_ACCEPTED) {
            if (web_server.timer.counting == 0) {
                web_server.timer.start();
            }
            int nBytes_received = web_server.receive();
            if (nBytes_received > 0) {
                web_server.timer.start();
                std::string data = "";
                for (int i = 0; i < web_server.message->length; i++) {
                    data += web_server.message->buffer[i];
                }

                std::cout << "Data received (" << nBytes_received << " bytes)" << std::endl;

                std::string reply = process_received_data(data);

                if (reply.length() != 0) {
                    std::cout << "Enviando resposta (" << reply.length() << " bytes)" << std::endl;
                    web_server.message->length = reply.length();
                    for (int i = 0; i < web_server.message->length; i++) {
                        web_server.message->buffer[i] = *(reply.c_str() + i);
                    }

                    if (web_server.get_status() == TCP_STATUS_ACCEPTED) {
                        web_server.transmit();
                    }
                }
            } else if (nBytes_received == 0 || (web_server.timer.get_elapsed_time_ms() >= web_server.timeout_ms && web_server.timeout_ms != -1)) {
                web_server.check_incomming_connection();
                if (web_server.get_status() == TCP_STATUS_ACCEPTED) {
                    web_server.timer.start();
                }
            }
        } else {
            web_server.check_incomming_connection();
            if (web_server.get_status() == TCP_STATUS_ACCEPTED) {
                std::cout << "New connection established!" << std::endl;
                web_server.timer.start();
            }
        }
        usleep(5000);
    }
}

std::string ATSC_Server::process_received_data(const std::string& data) {
    std::string buffer = data;
    std::string command, reply = "";
    size_t pos = std::string::npos;
    int commands_processed = 0;

    do {
        pos = buffer.find('\n');
        if (pos != std::string::npos) {
            command = buffer.substr(0, pos);
            if (!command.empty()) {
                std::string cmd_reply = config.process_command(command);
                reply += cmd_reply;
                commands_processed++;
            }
            buffer = buffer.substr(pos + 1, std::string::npos);
        }
    } while (pos != std::string::npos);

    std::cout << "Commands processed: " << commands_processed << std::endl;

    if (commands_processed > 0) {
        config.save();
        generator.generate_basic_multi_frame();
        generator.generate_detail_multi_frame();
        interpreter.run();
        calculator.write_frame_duration_file(calculator.calculate_frame_duration_ms());
    }

    return reply;
}
