#ifndef CALCULATOR_H
#define CALCULATOR_H

#include "model/atsc_config.h"
#include <cstdint>

namespace Calculator {
    int calculateL1DSizeBits(const AtscConfig& config);
    int calculateL1DSizeBytes(const AtscConfig& config);
    uint16_t calcTimeOffset(uint16_t bsr_coefficient, uint16_t usec, uint16_t nsec);
    uint32_t estimateL1DCells(uint32_t l1d_size_bytes, uint8_t fec_mode);
    int calculateSbsNullCells(const AtscConfig& config, int subframeIndex);
    int calculateSubframeDataCapacity(int fft_size, int reduced_carriers, int sp_pattern,
                                      int num_ofdm_symbols, bool sbs_first, bool sbs_last);
}

#endif
