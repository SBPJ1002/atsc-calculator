#ifndef L1_GENERATOR_H
#define L1_GENERATOR_H

#include "atsc_config.h"
#include "l1_calculator.h"
#include "binary_utils.h"
#include <string>

class L1_Generator {
public:
    L1_Generator(ATSC_Config& cfg, L1_Calculator& calc);
    void generate_basic_multi_frame();
    void generate_detail_multi_frame();

private:
    ATSC_Config& config;
    L1_Calculator& calculator;
};

#endif
