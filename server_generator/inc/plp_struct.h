#ifndef PLP_STRUCT
#define PLP_STRUCT

#include <string>
#include <vector>
#include <cstdint>

struct st_boostrap{
    int major_version = 137;
    int minor_version = 0;
    int bootstrap_symbol = 4;
    int ea_wakeup = 0;
    int system_bandwidth = 6;
    int bsr_coefficient = 0;
    int min_time_to_next = 50;
    int preamble_structure = 0;
};

struct PLP {
    int L1D_plp_id = 0;
    int L1D_plp_lls_flag = 0;
    int L1D_plp_layer = 0;
    int L1D_plp_start = 0;
    int L1D_plp_size = 0;
    int L1D_plp_scrambler_type;
    int L1D_plp_fec_type = 0;
    int L1D_plp_mod = 0;
    int L1D_plp_cod = 0;
    int L1D_plp_TI_mode = 0;
    int L1D_plp_fec_block_start;
    int L1D_plp_CTI_fec_block_start;
    int L1D_plp_num_channel_bonded;
    int L1D_plp_channel_bonding_format;
    int L1D_plp_bonded_rf_id;
    int L1D_plp_type = 0;
    int L1D_plp_num_subslices = 0;
    int L1D_plp_subslice_interval = 0;
    int L1D_plp_TI_extended_interleaving = 0;
    int L1D_plp_CTI_depth = 0;
    int L1D_plp_CTI_start_row;
    int L1D_plp_HTI_inter_subframe=0;
    int L1D_plp_HTI_num_ti_blocks=0;
    int L1D_plp_HTI_num_fec_blocks_max=0;
    int L1D_plp_HTI_num_fec_blocks=0;
    int L1D_plp_HTI_cell_interleaver=0;
    int L1D_plp_ldm_injection_level=0;

    int L1D_plp_mimo = 0;
    int L1D_plp_mimo_stream_combining = 0;
    int L1D_plp_mimo_IQ_interleaving = 0;
    int L1D_plp_mimo_PH = 0;

};

struct st_subframes{
    int L1B_first_sub_mimo = 0;
    int L1B_first_sub_miso = 0;
    int L1B_first_sub_fft_size = 0;
    int L1B_first_sub_reduced_carriers = 0;
    int L1B_first_sub_guard_interval = 0;
    int L1B_first_sub_num_ofdm_symbols = 0;
    int L1B_first_sub_scattered_pilot_pattern = 0;
    int L1B_first_sub_scattered_pilot_boost = 0;
    int L1B_first_sub_sbs_first = 0;
    int L1B_first_sub_sbs_last = 0;
    int L1B_first_sub_mimo_mixed = 1;
    long long L1B_crc;

    int L1D_mimo = 0;
    int L1D_mimo_mixed = 0;
    int L1D_miso = 0;
    int L1D_fft_size = 0;
    int L1D_reduced_carriers = 0;
    int L1D_guard_interval = 0;
    int L1D_num_ofdm_symbols = 0;
    int L1D_scattered_pilot_pattern = 0;
    int L1D_scattered_pilot_boost = 0;
    int L1D_sbs_first = 0;
    int L1D_sbs_last = 0;
    int L1D_subframe_multiplex = 0;
    int L1D_frequency_interleaver = 0;
    int L1D_sbs_null_cells = 0;

    std::vector<PLP> plps;
};

struct st_preamble{
    int L1B_version = 0;
    int L1B_mimo_scattered_pilot_encoding = 0;
    int L1B_lls_flag;
    int L1B_time_info_flag;
    int L1B_return_channel_flag;
    int L1B_papr_reduction;
    int L1B_frame_length_mode = 0;
    int L1B_frame_length = 0;
    int L1B_excess_samples_per_symbol;
    int L1B_time_offset;
    int L1B_additional_samples;
    int L1B_num_subframes = 1;
    int L1B_preamble_num_symbols = 0;
    int L1B_preamble_reduced_carriers;
    int L1B_L1_Detail_content_tag;
    int L1B_L1_Detail_size_bytes = 0;
    int L1B_L1_Detail_fec_type = 0;
    int L1B_L1_Detail_additional_parity_mode;
    int L1B_L1_Detail_total_cells;
    long long L1B_reserved = 0;

    int L1D_version = 0;
	int L1D_num_rf;
	int L1D_rf_id;
	int L1D_bonded_bsid;
	int L1D_bsid = 0;
	int reserved;
    int L1D_time_sec;
    int L1D_time_msec;
    int L1D_time_usec;
    int L1D_time_nsec;
    long long L1D_reserved = 0;
    int L1D_crc;

    std::vector<st_subframes> sub_frames;
};

#endif
