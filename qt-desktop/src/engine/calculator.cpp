#include "engine/calculator.h"
#include "engine/atsc_tables.h"
#include <cmath>
#include <algorithm>

namespace Calculator {

int calculateL1DSizeBits(const AtscConfig& config) {
    const auto& p = config.preamble;
    const auto& sfs = config.subframes;
    int totalBits = 0;
    totalBits += 4;
    totalBits += 3;

    for (int rf = 1; rf < p.l1d_num_rf; rf++) {
        totalBits += 16 + 3;
    }

    if (p.l1b_time_info_flag != 0) {
        totalBits += 32 + 10;
        if (p.l1b_time_info_flag != 1) {
            totalBits += 10;
            if (p.l1b_time_info_flag != 2)
                totalBits += 10;
        }
    }

    for (int i = 0; i < p.l1b_num_subframes && i < (int)sfs.size(); i++) {
        const auto& sf = sfs[i];
        if (i > 0) {
            totalBits += 1 + 2 + 2 + 3 + 4 + 11 + 5 + 3 + 1 + 1;
        }
        if (p.l1b_num_subframes - 1 > 0) totalBits += 1;
        totalBits += 1;

        bool hasSbs = (sf.sbs_first != 0 || sf.sbs_last != 0);
        if (hasSbs) totalBits += 13;

        totalBits += 6;

        for (int j = 0; j < (int)sf.plps.size(); j++) {
            const auto& plp = sf.plps[j];
            totalBits += 6 + 1 + 2 + 24 + 24 + 2 + 4;

            if (plp.fec_type <= 5)
                totalBits += 4 + 4;

            totalBits += 2;

            if (plp.ti_mode == 0) totalBits += 15;
            else if (plp.ti_mode == 1) totalBits += 22;

            if (p.l1d_num_rf > 0) {
                totalBits += 3;
                if (plp.num_channel_bonded > 0) {
                    totalBits += 2;
                    totalBits += 3 * plp.num_channel_bonded;
                }
            }

            bool isMimo = (sf.mimo == 1);
            if (isMimo) totalBits += 1 + 1 + 1;

            if (plp.layer == 0) {
                totalBits += 1;
                if (plp.type == 1) totalBits += 14 + 24;

                if ((plp.ti_mode == 1 || plp.ti_mode == 2) && plp.mod == 0)
                    totalBits += 1;

                if (plp.ti_mode == 1) {
                    totalBits += 3 + 11;
                } else if (plp.ti_mode == 2) {
                    totalBits += 1 + 4 + 12;
                    if (plp.hti_inter_subframe == 0)
                        totalBits += 12;
                    else
                        totalBits += 12 * (plp.hti_num_fec_blocks + 1);
                    totalBits += 1;
                }
            } else {
                totalBits += 5;
            }
        }
    }

    if (p.l1d_version == 1) totalBits += 16;

    for (int i = 0; i < p.l1b_num_subframes && i < (int)sfs.size(); i++) {
        const auto& sf = sfs[i];
        if (i > 0) totalBits += 1;

        bool shouldProcessMimo = (sf.mimo == 1 && sf.mimo_mixed == 1);
        if (shouldProcessMimo) {
            for (int j = 0; j < (int)sf.plps.size(); j++) {
                totalBits += 1;
                if (sf.plps[j].mimo == 1) totalBits += 1 + 1 + 1;
            }
        }
    }

    totalBits += 32;
    return totalBits;
}

int calculateL1DSizeBytes(const AtscConfig& config) {
    int bits = calculateL1DSizeBits(config);
    int bytes = (bits + 7) / 8;
    return std::max(bytes, 25);
}

uint16_t calcTimeOffset(uint16_t bsr_coefficient, uint16_t usec, uint16_t nsec) {
    uint64_t fs_hz = 384000ULL * ((uint64_t)bsr_coefficient + 16ULL);
    uint64_t delta_ns = (uint64_t)usec * 1000ULL + (uint64_t)nsec;
    uint64_t num = delta_ns * fs_hz + 500000000ULL;
    uint64_t den = 1000000000ULL;
    return (uint16_t)(num / den);
}

uint32_t estimateL1DCells(uint32_t l1d_size_bytes, uint8_t fec_mode) {
    double x = std::max(25u, l1d_size_bytes);
    double cells;
    switch (fec_mode) {
        case 0: cells = 48.4968 * x + 1574.5629; break;
        case 1: cells = 12.0 * x + 474.0; break;
        case 2: cells = 107.6082 * sqrt(x) + 43.9439; break;
        case 3: cells = 3.8087 * x + 242.6673; break;
        case 4: cells = 2.3328 * x + 145.3357; break;
        case 5: cells = 1.6913 * x + 81.2802; break;
        case 6: cells = 1.1896 * x + 54.9410; break;
        default: cells = 1.1896 * x + 54.9410; break;
    }
    return (uint32_t)std::round(cells);
}

int calculateSbsNullCells(const AtscConfig& config, int subframeIndex) {
    if (subframeIndex < 0 || subframeIndex >= (int)config.subframes.size()) return 0;
    const auto& sf = config.subframes[subframeIndex];

    if (sf.sbs_first == 0 && sf.sbs_last == 0) return 0;

    int totalCells = AtscTables::getSbsTotalDataCells(sf.fft_size, sf.reduced_carriers, sf.scattered_pilot_pattern);
    int activeCells = AtscTables::getSbsActiveCells(sf.fft_size, sf.reduced_carriers, sf.scattered_pilot_pattern, sf.scattered_pilot_boost);
    int nullCells = totalCells - activeCells;
    if (nullCells < 0) nullCells = 0;
    if (nullCells > 8191) nullCells = 8191;
    return nullCells;
}

int calculateSubframeDataCapacity(int fft_size, int reduced_carriers, int sp_pattern,
                                   int num_ofdm_symbols, bool sbs_first, bool sbs_last) {
    int cellsPerSymbol = AtscTables::getCellsPerSymbol(fft_size, reduced_carriers, sp_pattern);
    int sbsDataCells = AtscTables::getSbsTotalDataCells(fft_size, reduced_carriers, sp_pattern);
    int nDataSymbols = num_ofdm_symbols;
    if (sbs_first) nDataSymbols--;
    if (sbs_last) nDataSymbols--;
    int nSbsSymbols = (sbs_first ? 1 : 0) + (sbs_last ? 1 : 0);
    return (nDataSymbols * cellsPerSymbol) + (nSbsSymbols * sbsDataCells);
}

}
