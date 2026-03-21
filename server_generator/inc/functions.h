#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <fstream>
#include <sstream>
#include <chrono>
#include <map>
#include <iostream>
#include <ctime>
#include <bitset>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <ctime>
#include <random>
#include <cmath>
#include "tcp_socket.h"
#include "plp_struct.h"

inline void write_binary_to_file(std::ofstream& file, const std::string& binarySequence) {
    std::string paddedBinary = binarySequence;
    while (paddedBinary.length() % 8 != 0) {
        paddedBinary += "0";
    }

    for (size_t i = 0; i < paddedBinary.length(); i += 8) {
        std::string byteString = paddedBinary.substr(i, 8);
        unsigned char byte = 0;

        for (size_t j = 0; j < 8; j++) {
            if (byteString[j] == '1') {
                byte |= (1 << (7 - j));
            }
        }

        file.write(reinterpret_cast<const char*>(&byte), 1);
    }
}

std::string process_user_command(std::string command);
void load_config_from_file();
void save_config_to_file();
void auto_save_config();
void main_func();
std::string calculate_crc32(const std::string& binarySequence);
int generate_random(int min, int max);
int generate_number();
std::string to_binary(uint32_t value, int bits);
std::string to_binary(uint64_t value, int bits);
std::string to_binary(int value, int bits);
std::string to_binary(long long value, int bits);
std::string binary_string_to_hex(const std::string& binary);

void generate_L1_Basic();
void generate_L1_Detail();

void fill_L1D_time_information();
int calculate_L1D_size_bytes();

uint16_t calc_L1B_time_offset(uint16_t bsr_coefficient,uint16_t l1d_time_usec,uint16_t l1d_time_nsec);

void compute_and_set_L1B_time_offset();

uint32_t estimate_L1D_cells_from_regression(uint32_t l1d_size_bytes, uint8_t fec_mode);
uint32_t calc_L1B_L1_Detail_total_cells(uint32_t l1d_size_bytes, uint8_t fec_mode, uint8_t additional_parity_mode);

void compute_and_set_L1B_L1_Detail_total_cells(uint8_t fec_mode);

int calculate_sbs_null_cells(int subframe_index);
void compute_and_set_all_sbs_null_cells();

extern int global_frame_count;
uint16_t calculate_L1B_time_offset_for_frame(int frame_idx);
double calculate_frame_duration_ms();
void write_frame_duration_file(double total_duration_ms);
void generate_L1_Basic_multi_frame();
void generate_L1_Detail_multi_frame();

extern PLP plp;
extern st_boostrap bootstrap_info;
extern st_preamble preamble;
extern cl_tcp_socket web_server;



const std::string CONFIG_FILE = "config/config.conf";
const std::string BASIC_FILE_BINARY = "config/basic.bin";
const std::string BASIC_FILE_HEX = "config/basic.hex";
const std::string DETAIL_FILE_BINARY = "config/detail.bin";
const std::string DETAIL_FILE_HEX = "config/detail.hex";

#endif
