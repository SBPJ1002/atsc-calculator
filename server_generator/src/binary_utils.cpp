#include "binary_utils.h"
#include <sstream>
#include <bitset>
#include <iomanip>
#include <stdexcept>
#include <vector>

std::string BinaryUtils::to_binary(uint32_t value, int bits) {
    uint32_t mask;
    if (bits == 32) {
        mask = UINT32_MAX;
    } else {
        mask = (1U << bits) - 1;
    }
    value &= mask;
    std::string result = "";
    for (int i = bits - 1; i >= 0; i--) {
        result += ((value >> i) & 1U) ? '1' : '0';
    }

    return result;
}

std::string BinaryUtils::to_binary(uint64_t value, int bits) {
    std::string result = "";
    if (bits > 64) {
        bool propagate_ones = (value == UINT64_MAX);

        for (int i = bits - 1; i >= 64; i--) {
            result += propagate_ones ? '1' : '0';
        }

        for (int i = 63; i >= 0; i--) {
            result += ((value >> i) & 1ULL) ? '1' : '0';
        }
    }
    else {
        uint64_t mask;
        if (bits == 64) {
            mask = UINT64_MAX;
        } else {
            mask = (1ULL << bits) - 1;
        }
        value &= mask;

        for (int i = bits - 1; i >= 0; i--) {
            result += ((value >> i) & 1ULL) ? '1' : '0';
        }
    }

    return result;
}

std::string BinaryUtils::to_binary(int value, int bits) {
    return to_binary(static_cast<uint32_t>(value), bits);
}

std::string BinaryUtils::to_binary(long long value, int bits) {
    return to_binary(static_cast<uint64_t>(value), bits);
}

std::string BinaryUtils::binary_string_to_hex(const std::string& binary) {
    std::string result = "";
    std::string paddedBinary = binary;

    while (paddedBinary.length() % 4 != 0) {
        paddedBinary = "0" + paddedBinary;
    }

    for (size_t i = 0; i < paddedBinary.length(); i += 4) {
        std::string nibble = paddedBinary.substr(i, 4);
        int value = 0;
        for (char c : nibble) {
            value = (value << 1) | (c - '0');
        }
        const char hex_chars[] = "0123456789ABCDEF";
        result += hex_chars[value];
    }

    return result;
}

std::string BinaryUtils::calculate_crc32(const std::string& binarySequence) {
    const uint32_t polynomial = 0x00210801;
    std::vector<uint8_t> data;
    for (size_t i = 0; i < binarySequence.length(); i += 8) {
        std::string byteStr = binarySequence.substr(i, 8);

        while (byteStr.length() < 8) {
            byteStr += "0";
        }
        uint8_t byte = 0;
        for (int j = 0; j < 8; j++) {
            byte = (byte << 1) | (byteStr[j] - '0');
        }
        data.push_back(byte);
    }

    uint32_t crc = 0xFFFFFFFF;

    for (uint8_t byte : data) {
        crc ^= (uint32_t)byte << 24;

        for (int i = 0; i < 8; i++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ polynomial;
            } else {
                crc <<= 1;
            }
        }
    }

    std::bitset<32> crcBits(crc);
    return crcBits.to_string();
}

void BinaryUtils::write_binary_to_file(std::ofstream& file, const std::string& binarySequence) {
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

std::string BinaryUtils::hex_to_binary(const std::string& hex_value) {
    std::stringstream binary_stream;

    for (char hex_char : hex_value) {
        int decimalValue = std::stoi(std::string(1, hex_char), nullptr, 16);
        binary_stream << std::bitset<4>(decimalValue);
    }

    return binary_stream.str();
}

long long BinaryUtils::bin_to_decimal(const std::string& bin_value) {
    if (bin_value.length() > 63) {
        throw std::overflow_error("Binary value too large for long long (max 63 bits)");
    }
    return std::stoll(bin_value, nullptr, 2);
}

std::string BinaryUtils::bin_to_hex_large(const std::string& bin_value) {
    std::stringstream hex_stream;
    hex_stream << "0x";

    for (size_t i = 0; i < bin_value.length(); i += 4) {
        size_t remaining = bin_value.length() - i;
        size_t chunk_size = (remaining < 4) ? remaining : 4;
        std::string chunk = bin_value.substr(i, chunk_size);

        if (chunk.length() < 4) {
            chunk = std::string(4 - chunk.length(), '0') + chunk;
        }

        int val = std::stoi(chunk, nullptr, 2);
        hex_stream << std::hex << val;
    }

    return hex_stream.str();
}

std::string BinaryUtils::search_in_dicionary(const std::unordered_map<std::string, std::string>& dicionary, const std::string& key) {
    auto it = dicionary.find(key);
    if (it != dicionary.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string BinaryUtils::bytesToHex(const std::vector<unsigned char>& bytes) {
    std::stringstream ss;
    for (unsigned char byte : bytes) {
        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << (int)byte;
    }
    return ss.str();
}

std::string BinaryUtils::bytesToBinary(const std::vector<unsigned char>& bytes) {
    std::string result;
    for (unsigned char byte : bytes) {
        for (int i = 7; i >= 0; i--) {
            result += ((byte >> i) & 1) ? '1' : '0';
        }
    }
    return result;
}

int BinaryUtils::extractL1DetailSize(const std::vector<unsigned char>& basicData) {
    std::string binStr = bytesToBinary(basicData);

    char frameLengthMode = binStr[10];

    int pos;
    if (frameLengthMode == '0') {
        pos = 11 + 10 + 13;
    } else {
        pos = 11 + 16 + 7;
    }

    pos += 8 + 3 + 3 + 2;

    std::string sizeBits = binStr.substr(pos, 13);
    int l1dSize = std::stoi(sizeBits, nullptr, 2);

    return l1dSize;
}
