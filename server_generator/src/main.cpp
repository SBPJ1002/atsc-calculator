#include "functions.h"
#include "l1_interpreter.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    std::cout << "ATSC 3.0 Server" << std::endl;
    std::cout << "Starting server on port 6000..." << std::endl;

    preamble.sub_frames.resize(1);

    load_config_from_file();

    web_server.port = 6000;
    web_server.timeout_ms = 1000;

    if (web_server.init() != SUCCESS) {
        std::cout << "ERROR: Failed to initialize TCP sockets. Exiting..." << std::endl;
        exit(-1);
    }

    std::cout << "TCP server initialized successfully!" << std::endl;
    std::cout << "Configuration file: " << CONFIG_FILE << std::endl;
    std::cout << "Awaiting connections..." << std::endl;

    std::string user_command_buffer = "";
    std::vector<std::string> lines;

    while (1) {
        if (web_server.get_status() == TCP_STATUS_ACCEPTED) {
            if (web_server.timer.counting == 0) {
                web_server.timer.start();
            }
            int nBytes_received = web_server.receive();
            if (nBytes_received > 0) {
                web_server.timer.start();
                std::string user_command_buffer = "";
                std::string command, reply = "";
                for (int i = 0; i < web_server.message->length; i++) {
                    user_command_buffer += web_server.message->buffer[i];
                }

                std::cout << "Data received (" << nBytes_received << " bytes)" << std::endl;

                size_t pos = std::string::npos;
                int commands_processed = 0;
                do {
                    pos = user_command_buffer.find('\n');
                    if (pos != std::string::npos) {
                        command = user_command_buffer.substr(0, pos);
                        if (!command.empty()) {
                            std::string cmd_reply = process_user_command(command);
                            reply += cmd_reply;
                            commands_processed++;
                        }
                        user_command_buffer = user_command_buffer.substr(pos + 1, std::string::npos);
                    }
                } while (pos != std::string::npos);

                std::cout << "Commands processed: " << commands_processed << std::endl;

                if (commands_processed > 0) {
                    save_config_to_file();
                    generate_L1_Basic_multi_frame();
                    generate_L1_Detail_multi_frame();
                    run_interpreter();
                    write_frame_duration_file(calculate_frame_duration_ms());
                }

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
        usleep(1000);
    }

    return 0;
}
