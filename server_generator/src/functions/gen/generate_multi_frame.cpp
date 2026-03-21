#include "functions.h"

int global_frame_count = 1;

const int TIME_OFFSET_MODULO = 6912;
const int LDPC_SIZE_LONG = 64800;
const int LDPC_SIZE_SHORT = 16200;


int get_fft_size_samples(int fft_index) {
    switch (fft_index) {
        case 0: return 8192;
        case 1: return 16384;
        case 2: return 32768;
        default:
            std::cerr << "WARNING: Invalid FFT index " << fft_index << ", using 8K" << std::endl;
            return 8192;
    }
}

int get_guard_interval_samples(int gi_index, int fft_samples) {
    static const int gi_samples_8k[] = {
        0,
        192,
        384,
        512,
        768,
        1024,
        1536,
        2048,
        2432,
        3072,
        3648,
        4096,
		4864
    };

    static const int gi_absolute_samples[] = {
        0,
		192,
		384,
		512,
		768,
		1024,
		1536,
		2048,
		2432,
		3072,
		3648,
		4096,
		4864
    };

    if (gi_index >= 0 && gi_index <= 12) {
        return gi_absolute_samples[gi_index];
    }

    std::cerr << "WARNING: Invalid GI index " << gi_index << ", using 512" << std::endl;
    return 512;
}


int get_bits_per_cell(int mod_index) {
    switch (mod_index) {
        case 0:  return 2;
        case 1:  return 4;
        case 2:  return 6;
        case 3:  return 8;
        case 4:  return 10;
        case 5:  return 12;

        case 6:  return 2;
        case 7:  return 4;
        case 8:  return 6;
        case 9:  return 8;
        case 10: return 10;
        case 11: return 12;
        default:
            std::cerr << "WARNING: Invalid modulation index " << mod_index << ", using QPSK (2 bits)" << std::endl;
            return 2;
    }
}

int get_ldpc_size(int fec_type) {
    if (fec_type <= 5) {
        return LDPC_SIZE_LONG;
    }
    return LDPC_SIZE_SHORT;
}

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


static TimeOffsetParams g_time_offset_params = {0, 0, 0, 0, 0, false};
static std::map<int, FecBlockStartParams> g_fec_params;


void calculate_time_offset_params() {
    g_time_offset_params.fft_samples = get_fft_size_samples(
        preamble.sub_frames[0].L1B_first_sub_fft_size);

    g_time_offset_params.gi_samples = get_guard_interval_samples(
        preamble.sub_frames[0].L1B_first_sub_guard_interval,
        g_time_offset_params.fft_samples);

    g_time_offset_params.t_symbol = g_time_offset_params.fft_samples +
                                     g_time_offset_params.gi_samples;

    int num_ofdm_symbols = preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols;
    int preamble_symbols = preamble.L1B_preamble_num_symbols + 1;

    g_time_offset_params.n_total = num_ofdm_symbols + preamble_symbols;

    int64_t frame_duration_samples = (int64_t)g_time_offset_params.n_total *
                                      g_time_offset_params.t_symbol;
    g_time_offset_params.delta_t = frame_duration_samples % TIME_OFFSET_MODULO;

    g_time_offset_params.initialized = true;

    std::cout << "=== Time Offset Parameters ===" << std::endl;
    std::cout << "  FFT size: " << g_time_offset_params.fft_samples << " samples" << std::endl;
    std::cout << "  Guard interval: " << g_time_offset_params.gi_samples << " samples" << std::endl;
    std::cout << "  T_symbol: " << g_time_offset_params.t_symbol << " samples" << std::endl;
    std::cout << "  N_total (OFDM + preamble): " << g_time_offset_params.n_total << " symbols" << std::endl;
    std::cout << "  Delta_t: " << g_time_offset_params.delta_t << " samples" << std::endl;
}

void calculate_fec_block_start_params(int subframe_idx, int plp_idx) {
    int key = subframe_idx * 100 + plp_idx;

    FecBlockStartParams params;

    int ldpc_size    = get_ldpc_size(preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_fec_type);
    int bits_per_cell = get_bits_per_cell(preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mod);

    params.c_block  = ldpc_size / bits_per_cell;
    params.plp_size = preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_size;

    if (params.plp_size == 0) {
        params.delta_fbs = 0;
    } else {
        int remainder    = params.plp_size % params.c_block;
        params.delta_fbs = (params.c_block - remainder) % params.c_block;
    }

    params.initialized = true;
    g_fec_params[key] = params;
}

uint16_t calculate_L1B_time_offset_for_frame(int frame_idx) {
    if (!g_time_offset_params.initialized) {
        calculate_time_offset_params();
    }

    int64_t time_offset = ((int64_t)frame_idx * g_time_offset_params.delta_t) % TIME_OFFSET_MODULO;

    return (uint16_t)time_offset;
}

int calculate_L1D_plp_fec_block_start_for_frame(int frame_idx, int subframe_idx, int plp_idx) {
    int key = subframe_idx * 100 + plp_idx;

    if (g_fec_params.find(key) == g_fec_params.end() || !g_fec_params[key].initialized) {
        calculate_fec_block_start_params(subframe_idx, plp_idx);
    }

    FecBlockStartParams& params = g_fec_params[key];

    std::cout << "  [DEBUG] plp_size=" << params.plp_size
              << " c_block=" << params.c_block
              << " delta_fbs=" << params.delta_fbs
              << " frame_idx=" << frame_idx
              << " subframe=" << subframe_idx
              << " plp=" << plp_idx << std::endl;

    if (params.plp_size == 0) {
        int ti_mode = preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_TI_mode;
        if (ti_mode == 1) {
            return (1 << 22) - 1;
        } else {
            return (1 << 15) - 1;
        }
    }

    int64_t fec_block_start = ((int64_t)frame_idx * params.delta_fbs) % params.c_block;

    return (int)fec_block_start;
}

void reset_variable_field_params() {
    g_time_offset_params.initialized = false;
    g_fec_params.clear();
}

void validate_and_adjust_preamble_num_symbols() {
    // L1B_preamble_num_symbols is a 3-bit field (0-7), representing 1-8 symbols.
    // For the calculator, we allow up to 4 symbols (field values 0-3).
    if (preamble.L1B_preamble_num_symbols < 0) {
        std::cout << "[VALIDATION] L1B_preamble_num_symbols=" << preamble.L1B_preamble_num_symbols
                  << " is below minimum. Adjusting to 0 (1 symbol)." << std::endl;
        preamble.L1B_preamble_num_symbols = 0;
    } else if (preamble.L1B_preamble_num_symbols > 3) {
        std::cout << "[VALIDATION] L1B_preamble_num_symbols=" << preamble.L1B_preamble_num_symbols
                  << " exceeds maximum of 3 (4 symbols). Adjusting to 3." << std::endl;
        preamble.L1B_preamble_num_symbols = 3;
    }
}

double calculate_frame_duration_ms() {
    // Elementary period: Ts = 1 / Fs, where Fs = 384000 * (bsr_coefficient + 16)
    double Fs = 384000.0 * (bootstrap_info.bsr_coefficient + 16);
    double Ts = 1.0 / Fs;

    // Bootstrap duration: Fs_bs = system_bandwidth * 1024000 Hz
    // bootstrap_symbol field indicates N CAB symbols; total symbols in time domain = N + 1
    // First symbol (CAS): 2048 samples (no guard), remaining N symbols: 2048 + 512 = 2560 samples each
    double Fs_bs = bootstrap_info.system_bandwidth * 1024000.0;
    int bs_num_symbols = bootstrap_info.bootstrap_symbol + 1;
    int bs_total_samples = 2048 + (bs_num_symbols - 1) * (2048 + 512);
    double T_BS = bs_total_samples / Fs_bs;

    if (preamble.L1B_frame_length_mode == 0) {
        // Time-aligned: duration = L1B_frame_length * 5ms
        double duration_ms = preamble.L1B_frame_length * 5.0;
        std::cout << "=== Frame Duration (Time-Aligned) ===" << std::endl;
        std::cout << "  L1B_frame_length = " << preamble.L1B_frame_length << std::endl;
        std::cout << "  Duration = " << duration_ms << " ms" << std::endl;
        return duration_ms;
    }

    // Symbol-aligned (mode 1):
    // duration = T_BS + preamble_symbols * T_symbol_preamble + Σ(subframe_symbols * T_symbol_subframe) + additional_samples * Ts

    // Preamble symbols
    int preamble_num_symbols = preamble.L1B_preamble_num_symbols + 1;
    int preamble_fft = get_fft_size_samples(preamble.sub_frames[0].L1B_first_sub_fft_size);
    int preamble_gi = get_guard_interval_samples(preamble.sub_frames[0].L1B_first_sub_guard_interval, preamble_fft);
    double T_symbol_preamble = (preamble_fft + preamble_gi) * Ts;

    double duration_s = T_BS + preamble_num_symbols * T_symbol_preamble;

    std::cout << "=== Frame Duration (Symbol-Aligned) ===" << std::endl;
    std::cout << "  Fs = " << Fs << " Hz, Ts = " << Ts << " s" << std::endl;
    std::cout << "  Bootstrap: " << bs_num_symbols << " symbols, "
              << bs_total_samples << " samples @ " << Fs_bs / 1e6 << " MHz, T_BS = "
              << T_BS * 1000.0 << " ms" << std::endl;
    std::cout << "  Preamble: " << preamble_num_symbols << " symbols, T_symbol = "
              << T_symbol_preamble * 1000.0 << " ms" << std::endl;

    // Subframe OFDM symbols
    for (int i = 0; i < preamble.L1B_num_subframes; i++) {
        int fft_idx, gi_idx, num_ofdm;

        if (i == 0) {
            fft_idx = preamble.sub_frames[0].L1B_first_sub_fft_size;
            gi_idx = preamble.sub_frames[0].L1B_first_sub_guard_interval;
            num_ofdm = preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols;
        } else {
            fft_idx = preamble.sub_frames[i].L1D_fft_size;
            gi_idx = preamble.sub_frames[i].L1D_guard_interval;
            num_ofdm = preamble.sub_frames[i].L1D_num_ofdm_symbols;
        }

        int fft_samples = get_fft_size_samples(fft_idx);
        int gi_samples = get_guard_interval_samples(gi_idx, fft_samples);
        double T_symbol_sf = (fft_samples + gi_samples) * Ts;

        duration_s += num_ofdm * T_symbol_sf;

        std::cout << "  Subframe " << i << ": " << num_ofdm << " OFDM symbols, T_symbol = "
                  << T_symbol_sf * 1000.0 << " ms, subtotal = " << num_ofdm * T_symbol_sf * 1000.0 << " ms" << std::endl;
    }

    // Additional samples
    duration_s += preamble.L1B_additional_samples * Ts;

    double duration_ms = duration_s * 1000.0;
    std::cout << "  Additional samples: " << preamble.L1B_additional_samples << std::endl;
    std::cout << "  Total duration = " << duration_ms << " ms" << std::endl;

    return duration_ms;
}

void write_frame_duration_file(double total_duration_ms) {
    std::ofstream durationFile("config/log/frame_duration.txt");
    if (!durationFile.is_open()) {
        std::cerr << "ERROR: Failed to write config/log/frame_duration.txt" << std::endl;
        return;
    }

    durationFile << std::fixed << std::setprecision(6);
    durationFile << "frame_duration_ms=" << total_duration_ms << std::endl;

    // Per-subframe durations
    double Fs = 384000.0 * (bootstrap_info.bsr_coefficient + 16);
    double Ts = 1.0 / Fs;

    for (int i = 0; i < preamble.L1B_num_subframes; i++) {
        int fft_idx, gi_idx, num_ofdm;

        if (i == 0) {
            fft_idx = preamble.sub_frames[0].L1B_first_sub_fft_size;
            gi_idx = preamble.sub_frames[0].L1B_first_sub_guard_interval;
            num_ofdm = preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols;
        } else {
            fft_idx = preamble.sub_frames[i].L1D_fft_size;
            gi_idx = preamble.sub_frames[i].L1D_guard_interval;
            num_ofdm = preamble.sub_frames[i].L1D_num_ofdm_symbols;
        }

        int fft_samples = get_fft_size_samples(fft_idx);
        int gi_samples = get_guard_interval_samples(gi_idx, fft_samples);
        double sf_duration_ms = num_ofdm * (fft_samples + gi_samples) * Ts * 1000.0;

        durationFile << "subframe_" << i << "_duration_ms=" << sf_duration_ms << std::endl;
    }

    durationFile.close();
    std::cout << "Saved frame duration: config/log/frame_duration.txt" << std::endl;
}

void generate_L1_Basic_multi_frame() {
    std::string all_frames_binary = "";
    std::string all_frames_hex = "";

    validate_and_adjust_preamble_num_symbols();

    reset_variable_field_params();

    if (preamble.L1B_frame_length_mode == 1) {
        calculate_time_offset_params();
    }

    for (int frame_idx = 0; frame_idx < global_frame_count; frame_idx++) {
        preamble.L1B_L1_Detail_size_bytes = calculate_L1D_size_bytes();
        fill_L1D_time_information();

        if (preamble.L1B_frame_length_mode == 1) {
            preamble.L1B_time_offset = calculate_L1B_time_offset_for_frame(frame_idx);
        } else {
            compute_and_set_L1B_time_offset();
        }

        std::string frame_binarySequence = "";

        frame_binarySequence += to_binary(preamble.L1B_version, 3);
        frame_binarySequence += to_binary(preamble.L1B_mimo_scattered_pilot_encoding, 1);
        frame_binarySequence += to_binary(preamble.sub_frames[0].plps[0].L1D_plp_lls_flag, 1);
        frame_binarySequence += to_binary(preamble.L1B_time_info_flag, 2);
        frame_binarySequence += to_binary(preamble.L1B_return_channel_flag, 1);
        frame_binarySequence += to_binary(preamble.L1B_papr_reduction, 2);
        frame_binarySequence += to_binary(preamble.L1B_frame_length_mode, 1);

        if (preamble.L1B_frame_length_mode == 0){
            frame_binarySequence += to_binary(preamble.L1B_frame_length, 10);
            frame_binarySequence += to_binary(preamble.L1B_excess_samples_per_symbol, 13);
        } else {
            frame_binarySequence += to_binary(preamble.L1B_time_offset, 16);
            frame_binarySequence += to_binary(preamble.L1B_additional_samples, 7);
        }

        frame_binarySequence += to_binary(preamble.L1B_num_subframes-1, 8);
        frame_binarySequence += to_binary(preamble.L1B_preamble_num_symbols, 3);
        frame_binarySequence += to_binary(preamble.L1B_preamble_reduced_carriers, 3);
        frame_binarySequence += to_binary(preamble.L1B_L1_Detail_content_tag, 2);
        frame_binarySequence += to_binary(preamble.L1B_L1_Detail_size_bytes, 13);
        frame_binarySequence += to_binary(preamble.L1B_L1_Detail_fec_type, 3);
        frame_binarySequence += to_binary(preamble.L1B_L1_Detail_additional_parity_mode, 2);
        frame_binarySequence += to_binary(preamble.L1B_L1_Detail_total_cells, 19);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_mimo, 1);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_miso, 2);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_fft_size, 2);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_reduced_carriers, 3);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_guard_interval, 4);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols-1, 11);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_scattered_pilot_pattern, 5);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_scattered_pilot_boost, 3);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_sbs_first, 1);
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_sbs_last, 1);
        preamble.sub_frames[0].L1B_first_sub_mimo_mixed = 1;
        frame_binarySequence += to_binary(preamble.sub_frames[0].L1B_first_sub_mimo_mixed, 1);
        uint64_t L1B_reserved_value = (1ULL << 47) - 1;
        frame_binarySequence += to_binary(L1B_reserved_value, 47);

        std::string crc32_L1B = calculate_crc32(frame_binarySequence);
        frame_binarySequence += crc32_L1B;

        all_frames_binary += frame_binarySequence;

        std::string frame_hex = binary_string_to_hex(frame_binarySequence);
        all_frames_hex += "Frame #" + std::to_string(frame_idx + 1) + " (" + std::to_string(frame_hex.length() / 2) + " bytes)\n";
        all_frames_hex += frame_hex;
        if (frame_idx < global_frame_count - 1) {
            all_frames_hex += "\n";
        }

        std::cout << "Frame " << (frame_idx + 1) << " L1-Basic: time_offset="
                  << preamble.L1B_time_offset << " (" << frame_hex.length() / 2 << " bytes)" << std::endl;
    }

    std::ofstream basicFile(BASIC_FILE_BINARY, std::ios::binary);
    if (basicFile.is_open()) {
        write_binary_to_file(basicFile, all_frames_binary);
        basicFile.close();
        std::cout << "\nSaved L1-Basic multi-frame binary file: " << BASIC_FILE_BINARY << std::endl;
        std::cout << "Total frames: " << global_frame_count << std::endl;
        std::cout << "Total size: " << all_frames_binary.length() << " bits ("
                  << all_frames_binary.length() / 8 << " bytes)" << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save L1-Basic: " << BASIC_FILE_BINARY << std::endl;
    }

    std::ofstream basicFileHex(BASIC_FILE_HEX);
    if (basicFileHex.is_open()) {
        basicFileHex << all_frames_hex;
        basicFileHex.close();
        std::cout << "Saved hexadecimal multi-frame L1-Basic: " << BASIC_FILE_HEX << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save hexadecimal L1-Basic: " << BASIC_FILE_HEX << std::endl;
    }

}

void generate_L1_Detail_multi_frame() {
    std::string all_frames_binary = "";
    std::string all_frames_hex = "";

    for (int i = 0; i < preamble.L1B_num_subframes; i++) {
        for (size_t j = 0; j < preamble.sub_frames[i].plps.size(); j++) {
            calculate_fec_block_start_params(i, j);
        }
    }

    for (int frame_idx = 0; frame_idx < global_frame_count; frame_idx++) {
        std::string frame_binarySequence = "";

        compute_and_set_L1B_L1_Detail_total_cells(preamble.L1B_L1_Detail_fec_type);
        compute_and_set_all_sbs_null_cells();


        frame_binarySequence += to_binary(preamble.L1D_version, 4);
        frame_binarySequence += to_binary(preamble.L1D_num_rf, 3);

        for (int L1D_rf_id = 1; L1D_rf_id < preamble.L1D_num_rf; L1D_rf_id++) {
            frame_binarySequence += to_binary(preamble.L1D_bonded_bsid, 16);
            frame_binarySequence += to_binary(preamble.reserved, 3);
        }

        if (preamble.L1B_time_info_flag != 0) {
            frame_binarySequence += to_binary(preamble.L1D_time_sec, 32);
            frame_binarySequence += to_binary(preamble.L1D_time_msec, 10);

            if (preamble.L1B_time_info_flag != 1) {
                frame_binarySequence += to_binary(preamble.L1D_time_usec, 10);
                if (preamble.L1B_time_info_flag != 2) {
                    frame_binarySequence += to_binary(preamble.L1D_time_nsec, 10);
                }
            }
        }

        std::cout << "Frame " << (frame_idx + 1) << " L1-Detail:" << std::endl;

        for (int i = 0; i < preamble.L1B_num_subframes; i++) {
            if (i > 0) {
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_mimo, 1);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_miso, 2);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_fft_size, 2);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_reduced_carriers, 3);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_guard_interval, 4);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_num_ofdm_symbols, 11);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_scattered_pilot_pattern, 5);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_scattered_pilot_boost, 3);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_sbs_first, 1);
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_sbs_last, 1);
            }

            if (preamble.L1B_num_subframes - 1 > 0) {
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_subframe_multiplex, 1);
            }

            frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_frequency_interleaver, 1);

            if (((i == 0) && (preamble.sub_frames[0].L1B_first_sub_sbs_first || preamble.sub_frames[0].L1B_first_sub_sbs_last)) ||
                ((i > 0) && (preamble.sub_frames[i].L1D_sbs_first || preamble.sub_frames[i].L1D_sbs_last))) {
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_sbs_null_cells, 13);
            }

            frame_binarySequence += to_binary(preamble.sub_frames[i].plps.size()-1, 6);

            for (size_t j = 0; j < preamble.sub_frames[i].plps.size(); j++) {
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_id, 6);
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_lls_flag, 1);
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_layer, 2);
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_start, 24);
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_size, 24);
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_scrambler_type, 2);
                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_fec_type, 4);

                if(preamble.sub_frames[i].plps[j].L1D_plp_fec_type <= 5){
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mod, 4);
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_cod, 4);
                }

                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_TI_mode, 2);

                int plp_fec_block_start = calculate_L1D_plp_fec_block_start_for_frame(frame_idx, i, j);

                if(preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 0){
                    frame_binarySequence += to_binary(plp_fec_block_start, 15);
                    std::cout << "  Subframe " << i << " PLP " << j
                              << " fec_block_start = " << plp_fec_block_start << std::endl;
                } else if(preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) {
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_fec_block_start, 22);
                }

                if(preamble.L1D_num_rf > 0){
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded, 3);
                    if(preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded > 0){
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_channel_bonding_format, 2);
                        for (int k = 0; k < preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded; k++) {
                            frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_bonded_rf_id + k, 3);
                        }
                    }
                }

                if((i == 0 && preamble.sub_frames[0].L1B_first_sub_mimo == 1) ||
                   (i > 0 && preamble.sub_frames[i].L1D_mimo == 1)){
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo_stream_combining, 1);
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo_IQ_interleaving, 1);
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo_PH, 1);
                }

                if(preamble.sub_frames[i].plps[j].L1D_plp_layer == 0){
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_type, 1);

                    if(preamble.sub_frames[i].plps[j].L1D_plp_type == 1){
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_num_subslices, 14);
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_subslice_interval, 24);
                    }

                    if(((preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) ||
                        (preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 2)) &&
                       (preamble.sub_frames[i].plps[j].L1D_plp_mod == 0)) {
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_TI_extended_interleaving, 1);
                    }

                    if(preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1){
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_CTI_depth, 3);
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_CTI_start_row, 11);
                    } else if(preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 2){
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe, 1);
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_ti_blocks, 4);
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks_max, 12);

                        if(preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe == 0){
                            frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks, 12);
                        } else {
                            for (int k = 0; k <= preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks; k++) {
                                frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks, 12);
                            }
                        }
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_HTI_cell_interleaver, 1);
                    }
                } else {
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_ldm_injection_level, 5);
                }
            }
        }

        if(preamble.L1D_version == 1){
            frame_binarySequence += to_binary(preamble.L1D_bsid, 16);
        }

        for(int i=0; i<preamble.L1B_num_subframes;i++){
            if(i > 0){
                frame_binarySequence += to_binary(preamble.sub_frames[i].L1D_mimo_mixed, 1);
            }

            if ((i == 0 && preamble.sub_frames[0].L1B_first_sub_mimo == 1 && preamble.sub_frames[0].L1B_first_sub_mimo_mixed == 1) ||
                (i > 0 && preamble.sub_frames[i].L1D_mimo == 1 && preamble.sub_frames[i].L1D_mimo_mixed == 1)) {
                for (size_t j = 0; j < preamble.sub_frames[i].plps.size(); j++) {
                    frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo, 1);

                    if (preamble.sub_frames[i].plps[j].L1D_plp_mimo == 1) {
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo_stream_combining, 1);
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo_IQ_interleaving, 1);
                        frame_binarySequence += to_binary(preamble.sub_frames[i].plps[j].L1D_plp_mimo_PH, 1);
                    }
                }
            }
        }

        int current_bits = frame_binarySequence.size();
        int required_total_bits = preamble.L1B_L1_Detail_size_bytes * 8;
        int crc_bits = 32;
        int L1D_reserved_len = required_total_bits - current_bits - crc_bits;

        if (L1D_reserved_len < 0) {
            L1D_reserved_len = 0;
        }

        if (L1D_reserved_len > 0) {
            if (preamble.L1D_version == 2) {
                frame_binarySequence += to_binary(preamble.L1D_bsid, 16);
                if (L1D_reserved_len > 16) {
                    std::string remaining_reserved(L1D_reserved_len - 16, '1');
                    frame_binarySequence += remaining_reserved;
                }
            } else {
                std::string L1D_reserved_bits(L1D_reserved_len, '1');
                frame_binarySequence += L1D_reserved_bits;
            }
        }

        std::string crc32 = calculate_crc32(frame_binarySequence);
        frame_binarySequence += crc32;

        std::string frame_hex = binary_string_to_hex(frame_binarySequence);

        int final_bytes = frame_hex.length() / 2;
        if (final_bytes != preamble.L1B_L1_Detail_size_bytes) {
            std::cerr << "WARNING Frame " << (frame_idx + 1) << ": Final L1D size (" << final_bytes
                      << " bytes) different than expected ("
                      << preamble.L1B_L1_Detail_size_bytes << " bytes)" << std::endl;
        }

        all_frames_binary += frame_binarySequence;

        all_frames_hex += "Frame #" + std::to_string(frame_idx + 1) + " (" + std::to_string(final_bytes) + " bytes)\n";
        all_frames_hex += frame_hex;
        if (frame_idx < global_frame_count - 1) {
            all_frames_hex += "\n";
        }
    }

    std::ofstream detailFile(DETAIL_FILE_BINARY, std::ios::binary);
    if (detailFile.is_open()) {
        write_binary_to_file(detailFile, all_frames_binary);
        detailFile.close();
        std::cout << "\nL1-Detail multi-frame binary file saved: " << DETAIL_FILE_BINARY << std::endl;
        std::cout << "Total frames: " << global_frame_count << std::endl;
        std::cout << "Total size: " << all_frames_binary.length() << " bits ("
                  << all_frames_binary.length() / 8 << " bytes)" << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save L1-Detail: " << DETAIL_FILE_BINARY << std::endl;
    }

    std::ofstream detailFileHex(DETAIL_FILE_HEX);
    if (detailFileHex.is_open()) {
        detailFileHex << all_frames_hex;
        detailFileHex.close();
        std::cout << "Saved hexadecimal multi-frame L1-Detail: " << DETAIL_FILE_HEX << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save hexadecimal L1-Detail: " << DETAIL_FILE_HEX << std::endl;
    }
}
