#ifndef L1_CALCULATOR_H
#define L1_CALCULATOR_H

#include "atsc_config.h"
#include <map>
#include <cstdint>

class L1_Calculator {
public:
    L1_Calculator(ATSC_Config& cfg);

    void fill_L1D_time_information();
    int calculate_L1D_size_bits();
    int calculate_L1D_size_bytes();

    uint16_t calc_L1B_time_offset(uint16_t bsr_coefficient, uint16_t l1d_time_usec, uint16_t l1d_time_nsec);
    void compute_and_set_L1B_time_offset();

    uint32_t estimate_L1D_cells_from_regression(uint32_t l1d_size_bytes, uint8_t fec_mode);
    uint32_t calc_L1B_L1_Detail_total_cells(uint32_t l1d_size_bytes, uint8_t fec_mode, uint8_t additional_parity_mode);
    void compute_and_set_L1B_L1_Detail_total_cells(uint8_t fec_mode);

    int calculate_sbs_null_cells(int subframe_index);
    void compute_and_set_all_sbs_null_cells();

    uint16_t calculate_L1B_time_offset_for_frame(int frame_idx);
    double calculate_frame_duration_ms();
    void write_frame_duration_file(double total_duration_ms);

    void validate_and_adjust_preamble_num_symbols();
    void reset_variable_field_params();
    int calculate_L1D_plp_fec_block_start_for_frame(int frame_idx, int subframe_idx, int plp_idx);
    void calculate_time_offset_params();
    void calculate_fec_block_start_params(int subframe_idx, int plp_idx);

private:
    ATSC_Config& config;

    // Helper functions
    int get_fft_size_samples(int fft_index);
    int get_guard_interval_samples(int gi_index, int fft_samples);
    int get_bits_per_cell(int mod_index);
    int get_ldpc_size(int fec_type);
    uint8_t count_total_plps();

    // Internal state for multi-frame calculations
    struct TimeOffsetParams {
        int fft_samples;
        int gi_samples;
        int t_symbol;
        int n_total;
        int delta_t;
        bool initialized;
    };

    struct FecBlockStartParams {
        int c_block;
        int plp_size;
        int delta_fbs;
        bool initialized;
    };

    TimeOffsetParams time_offset_params;
    std::map<int, FecBlockStartParams> fec_params;
};

#endif
