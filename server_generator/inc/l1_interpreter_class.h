#ifndef L1_INTERPRETER_CLASS_H
#define L1_INTERPRETER_CLASS_H

#include "atsc_config.h"
#include "binary_utils.h"
#include <string>
#include <vector>
#include <unordered_map>

class L1_Interpreter {
public:
    L1_Interpreter(ATSC_Config& cfg, const std::string& log_dir = "config/log");
    void run();
    void interpret(const std::string& hex_value_detail, const std::string& hex_value_basic, const std::string& filename);

private:
    ATSC_Config& config;
    std::string log_dir;
    size_t cursor;
    size_t elements_processed;
    static const int L1_BASIC_SIZE_BYTES = 25;

    std::string process_segment(const std::string& array, size_t size, bool reset = false);
    void clearLogDirectory(const std::string& logDir);
};

#endif
