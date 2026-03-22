#ifndef L1_INTERPRETER_H
#define L1_INTERPRETER_H

#include <string>
#include <vector>
#include <unordered_map>

const std::string LOG_DIR = "config/log";

void l1_interpreter(const std::string& hex_value_detail, const std::string& hex_value_basic, const std::string& filename);

void run_interpreter();

std::string hex_to_binary(const std::string& hex_value);
long long bin_to_decimal(const std::string& bin_value);
std::string bin_to_hex_large(const std::string& bin_value);
std::string search_in_dicionary(const std::unordered_map<std::string, std::string>& dicionary, const std::string& key);

std::string bytesToHex(const std::vector<unsigned char>& bytes);
std::string bytesToBinary(const std::vector<unsigned char>& bytes);
int extractL1DetailSize(const std::vector<unsigned char>& basicData);

#endif
