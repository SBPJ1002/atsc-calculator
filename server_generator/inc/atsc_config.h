#ifndef ATSC_CONFIG_H
#define ATSC_CONFIG_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include "plp_struct.h"

class ATSC_Config {
public:
    std::string config_file;
    std::string basic_file_binary;
    std::string basic_file_hex;
    std::string detail_file_binary;
    std::string detail_file_hex;

    st_boostrap bootstrap;
    st_preamble preamble;
    PLP plp;
    int frame_count;

    ATSC_Config(const std::string& file = "config/config.conf");

    void load();
    void save();
    std::string process_command(const std::string& command);
};

#endif
