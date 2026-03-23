#ifndef ATSC_CONFIG_H
#define ATSC_CONFIG_H

#include <vector>
#include <cstdint>

struct PlpConfig {
    int id = 0;
    int lls_flag = 0;
    int layer = 0;
    int alloc_mode = 0; // 0=auto, 1=manual
    int start = 0;
    int size = 0;
    int scrambler_type = 0;
    int fec_type = 0;
    int mod = 0;
    int cod = 0;
    int ti_mode = 0;
    int fec_block_start = 0;
    int cti_fec_block_start = 0;
    int num_channel_bonded = 0;
    int channel_bonding_format = 0;
    int bonded_rf_id = 0;
    int type = 0;
    int num_subslices = 0;
    int subslice_interval = 0;
    int ti_extended_interleaving = 0;
    int cti_depth = 0;
    int cti_start_row = 0;
    int hti_inter_subframe = 0;
    int hti_num_ti_blocks = 0;
    int hti_num_fec_blocks_max = 0;
    int hti_num_fec_blocks = 0;
    int hti_cell_interleaver = 0;
    int ldm_injection_level = 0;
    int mimo = 0;
    int mimo_stream_combining = 0;
    int mimo_iq_interleaving = 0;
    int mimo_ph = 0;
};

struct SubframeConfig {
    int mimo = 0;
    int mimo_mixed = 1;
    int miso = 0;
    int fft_size = 0;
    int reduced_carriers = 0;
    int guard_interval = 0;
    int num_ofdm_symbols = 0;
    int scattered_pilot_pattern = 0;
    int scattered_pilot_boost = 0;
    int sbs_first = 0;
    int sbs_last = 0;
    int subframe_multiplex = 0;
    int frequency_interleaver = 0;
    int sbs_null_cells = 0;

    std::vector<PlpConfig> plps;
};

struct BootstrapConfig {
    int major_version = 137;
    int minor_version = 0;
    int bootstrap_symbol = 4;
    int ea_wakeup = 0;
    int system_bandwidth = 6;
    int bsr_coefficient = 0;
    int min_time_to_next = 50;
    int preamble_structure = 0;
    int number_of_frames = 10;
};

struct PreambleConfig {
    int l1b_version = 0;
    int l1b_mimo_scattered_pilot_encoding = 0;
    int l1b_lls_flag = 0;
    int l1b_time_info_flag = 0;
    int l1b_return_channel_flag = 0;
    int l1b_papr_reduction = 0;
    int l1b_frame_length_mode = 0;
    int l1b_frame_length = 0;
    int l1b_excess_samples_per_symbol = 0;
    int l1b_time_offset = 0;
    int l1b_additional_samples = 0;
    int l1b_num_subframes = 1;
    int l1b_preamble_num_symbols = 0;
    int l1b_preamble_reduced_carriers = 0;
    int l1b_detail_content_tag = 0;
    int l1b_detail_size_bytes = 25;
    int l1b_detail_fec_type = 0;
    int l1b_detail_additional_parity_mode = 0;
    int l1b_detail_total_cells = 0;

    int l1d_version = 0;
    int l1d_bsid = 0;
    int l1d_num_rf = 0;
    int l1d_rf_id = 0;
    int l1d_bonded_bsid = 0;
    int l1d_time_sec = 0;
    int l1d_time_msec = 0;
    int l1d_time_usec = 0;
    int l1d_time_nsec = 0;
};

struct AtscConfig {
    BootstrapConfig bootstrap;
    PreambleConfig preamble;
    std::vector<SubframeConfig> subframes;

    void ensureSubframes() {
        int count = preamble.l1b_num_subframes;
        if (count < 1) count = 1;
        while ((int)subframes.size() < count) {
            SubframeConfig sf;
            PlpConfig plp;
            sf.plps.push_back(plp);
            subframes.push_back(sf);
        }
        subframes.resize(count);
    }
};

#endif
