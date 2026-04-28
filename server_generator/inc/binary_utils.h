#ifndef BINARY_UTILS_H
#define BINARY_UTILS_H

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <unordered_map>

class BinaryUtils {
public:
    static std::string to_binary(uint32_t value, int bits);
    static std::string to_binary(uint64_t value, int bits);
    static std::string to_binary(int value, int bits);
    static std::string to_binary(long long value, int bits);

    static std::string binary_string_to_hex(const std::string& binary);
    static std::string calculate_crc32(const std::string& binarySequence);
    static void write_binary_to_file(std::ofstream& file, const std::string& binarySequence);

    static std::string hex_to_binary(const std::string& hex_value);
    static long long bin_to_decimal(const std::string& bin_value);
    static std::string bin_to_hex_large(const std::string& bin_value);

    static std::string search_in_dicionary(const std::unordered_map<std::string, std::string>& dicionary, const std::string& key);

    static std::string bytesToHex(const std::vector<unsigned char>& bytes);
    static std::string bytesToBinary(const std::vector<unsigned char>& bytes);
    static int extractL1DetailSize(const std::vector<unsigned char>& basicData);
};

#endif
