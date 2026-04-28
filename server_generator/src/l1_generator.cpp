#include "l1_generator.h"
#include <fstream>
#include <iostream>
#include <iomanip>

L1_Generator::L1_Generator(ATSC_Config& cfg, L1_Calculator& calc) : config(cfg), calculator(calc) {}

void L1_Generator::generate_basic_multi_frame() {
    std::string all_frames_binary = "";
    std::string all_frames_hex = "";

    calculator.validate_and_adjust_preamble_num_symbols();

    calculator.reset_variable_field_params();

    if (config.preamble.L1B_frame_length_mode == 1) {
        calculator.calculate_time_offset_params();
    }

    for (int frame_idx = 0; frame_idx < config.frame_count; frame_idx++) {
        config.preamble.L1B_L1_Detail_size_bytes = calculator.calculate_L1D_size_bytes();
        calculator.fill_L1D_time_information();

        if (config.preamble.L1B_frame_length_mode == 1) {
            config.preamble.L1B_time_offset = calculator.calculate_L1B_time_offset_for_frame(frame_idx);
        } else {
            calculator.compute_and_set_L1B_time_offset();
        }

        std::string frame_binarySequence = "";

        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_version, 3);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_mimo_scattered_pilot_encoding, 1);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].plps[0].L1D_plp_lls_flag, 1);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_time_info_flag, 2);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_return_channel_flag, 1);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_papr_reduction, 2);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_frame_length_mode, 1);

        if (config.preamble.L1B_frame_length_mode == 0){
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_frame_length, 10);
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_excess_samples_per_symbol, 13);
        } else {
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_time_offset, 16);
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_additional_samples, 7);
        }

        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_num_subframes-1, 8);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_preamble_num_symbols, 3);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_preamble_reduced_carriers, 3);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_L1_Detail_content_tag, 2);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_L1_Detail_size_bytes, 13);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_L1_Detail_fec_type, 3);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_L1_Detail_additional_parity_mode, 2);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1B_L1_Detail_total_cells, 19);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_mimo, 1);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_miso, 2);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_fft_size, 2);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_reduced_carriers, 3);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_guard_interval, 4);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols-1, 11);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_scattered_pilot_pattern, 5);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_scattered_pilot_boost, 3);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_sbs_first, 1);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_sbs_last, 1);
        config.preamble.sub_frames[0].L1B_first_sub_mimo_mixed = 1;
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[0].L1B_first_sub_mimo_mixed, 1);
        uint64_t L1B_reserved_value = (1ULL << 47) - 1;
        frame_binarySequence += BinaryUtils::to_binary(L1B_reserved_value, 47);

        std::string crc32_L1B = BinaryUtils::calculate_crc32(frame_binarySequence);
        frame_binarySequence += crc32_L1B;

        all_frames_binary += frame_binarySequence;

        std::string frame_hex = BinaryUtils::binary_string_to_hex(frame_binarySequence);
        all_frames_hex += "Frame #" + std::to_string(frame_idx + 1) + " (" + std::to_string(frame_hex.length() / 2) + " bytes)\n";
        all_frames_hex += frame_hex;
        if (frame_idx < config.frame_count - 1) {
            all_frames_hex += "\n";
        }

        std::cout << "Frame " << (frame_idx + 1) << " L1-Basic: time_offset="
                  << config.preamble.L1B_time_offset << " (" << frame_hex.length() / 2 << " bytes)" << std::endl;
    }

    std::ofstream basicFile(config.basic_file_binary, std::ios::binary);
    if (basicFile.is_open()) {
        BinaryUtils::write_binary_to_file(basicFile, all_frames_binary);
        basicFile.close();
        std::cout << "\nSaved L1-Basic multi-frame binary file: " << config.basic_file_binary << std::endl;
        std::cout << "Total frames: " << config.frame_count << std::endl;
        std::cout << "Total size: " << all_frames_binary.length() << " bits ("
                  << all_frames_binary.length() / 8 << " bytes)" << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save L1-Basic: " << config.basic_file_binary << std::endl;
    }

    std::ofstream basicFileHex(config.basic_file_hex);
    if (basicFileHex.is_open()) {
        basicFileHex << all_frames_hex;
        basicFileHex.close();
        std::cout << "Saved hexadecimal multi-frame L1-Basic: " << config.basic_file_hex << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save hexadecimal L1-Basic: " << config.basic_file_hex << std::endl;
    }

}

void L1_Generator::generate_detail_multi_frame() {
    std::string all_frames_binary = "";
    std::string all_frames_hex = "";

    for (int i = 0; i < config.preamble.L1B_num_subframes; i++) {
        for (size_t j = 0; j < config.preamble.sub_frames[i].plps.size(); j++) {
            calculator.calculate_fec_block_start_params(i, j);
        }
    }

    for (int frame_idx = 0; frame_idx < config.frame_count; frame_idx++) {
        std::string frame_binarySequence = "";

        calculator.compute_and_set_L1B_L1_Detail_total_cells(config.preamble.L1B_L1_Detail_fec_type);
        calculator.compute_and_set_all_sbs_null_cells();

        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_version, 4);
        frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_num_rf, 3);

        for (int L1D_rf_id = 1; L1D_rf_id < config.preamble.L1D_num_rf; L1D_rf_id++) {
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_bonded_bsid, 16);
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.reserved, 3);
        }

        if (config.preamble.L1B_time_info_flag != 0) {
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_time_sec, 32);
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_time_msec, 10);

            if (config.preamble.L1B_time_info_flag != 1) {
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_time_usec, 10);
                if (config.preamble.L1B_time_info_flag != 2) {
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_time_nsec, 10);
                }
            }
        }

        std::cout << "Frame " << (frame_idx + 1) << " L1-Detail:" << std::endl;

        for (int i = 0; i < config.preamble.L1B_num_subframes; i++) {
            if (i > 0) {
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_mimo, 1);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_miso, 2);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_fft_size, 2);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_reduced_carriers, 3);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_guard_interval, 4);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_num_ofdm_symbols, 11);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_scattered_pilot_pattern, 5);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_scattered_pilot_boost, 3);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_sbs_first, 1);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_sbs_last, 1);
            }

            if (config.preamble.L1B_num_subframes - 1 > 0) {
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_subframe_multiplex, 1);
            }

            frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_frequency_interleaver, 1);

            if (((i == 0) && (config.preamble.sub_frames[0].L1B_first_sub_sbs_first || config.preamble.sub_frames[0].L1B_first_sub_sbs_last)) ||
                ((i > 0) && (config.preamble.sub_frames[i].L1D_sbs_first || config.preamble.sub_frames[i].L1D_sbs_last))) {
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_sbs_null_cells, 13);
            }

            frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps.size()-1, 6);

            for (size_t j = 0; j < config.preamble.sub_frames[i].plps.size(); j++) {
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_id, 6);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_lls_flag, 1);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_layer, 2);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_start, 24);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_size, 24);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_scrambler_type, 2);
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_fec_type, 4);

                if(config.preamble.sub_frames[i].plps[j].L1D_plp_fec_type <= 5){
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mod, 4);
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_cod, 4);
                }

                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode, 2);

                int plp_fec_block_start = calculator.calculate_L1D_plp_fec_block_start_for_frame(frame_idx, i, j);

                if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 0){
                    frame_binarySequence += BinaryUtils::to_binary(plp_fec_block_start, 15);
                    std::cout << "  Subframe " << i << " PLP " << j
                              << " fec_block_start = " << plp_fec_block_start << std::endl;
                } else if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) {
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_fec_block_start, 22);
                }

                if(config.preamble.L1D_num_rf > 0){
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded, 3);
                    if(config.preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded > 0){
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_channel_bonding_format, 2);
                        for (int k = 0; k < config.preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded; k++) {
                            frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_bonded_rf_id + k, 3);
                        }
                    }
                }

                if((i == 0 && config.preamble.sub_frames[0].L1B_first_sub_mimo == 1) ||
                   (i > 0 && config.preamble.sub_frames[i].L1D_mimo == 1)){
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo_stream_combining, 1);
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo_IQ_interleaving, 1);
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo_PH, 1);
                }

                if(config.preamble.sub_frames[i].plps[j].L1D_plp_layer == 0){
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_type, 1);

                    if(config.preamble.sub_frames[i].plps[j].L1D_plp_type == 1){
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_num_subslices, 14);
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_subslice_interval, 24);
                    }

                    if(((config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) ||
                        (config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 2)) &&
                       (config.preamble.sub_frames[i].plps[j].L1D_plp_mod == 0)) {
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_extended_interleaving, 1);
                    }

                    if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1){
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_CTI_depth, 3);
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_CTI_start_row, 11);
                    } else if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 2){
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe, 1);
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_ti_blocks, 4);
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks_max, 12);

                        if(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe == 0){
                            frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks, 12);
                        } else {
                            for (int k = 0; k <= config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks; k++) {
                                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks, 12);
                            }
                        }
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_cell_interleaver, 1);
                    }
                } else {
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_ldm_injection_level, 5);
                }
            }
        }

        if(config.preamble.L1D_version == 1){
            frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_bsid, 16);
        }

        for(int i=0; i<config.preamble.L1B_num_subframes;i++){
            if(i > 0){
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].L1D_mimo_mixed, 1);
            }

            if ((i == 0 && config.preamble.sub_frames[0].L1B_first_sub_mimo == 1 && config.preamble.sub_frames[0].L1B_first_sub_mimo_mixed == 1) ||
                (i > 0 && config.preamble.sub_frames[i].L1D_mimo == 1 && config.preamble.sub_frames[i].L1D_mimo_mixed == 1)) {
                for (size_t j = 0; j < config.preamble.sub_frames[i].plps.size(); j++) {
                    frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo, 1);

                    if (config.preamble.sub_frames[i].plps[j].L1D_plp_mimo == 1) {
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo_stream_combining, 1);
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo_IQ_interleaving, 1);
                        frame_binarySequence += BinaryUtils::to_binary(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo_PH, 1);
                    }
                }
            }
        }

        int current_bits = frame_binarySequence.size();
        int required_total_bits = config.preamble.L1B_L1_Detail_size_bytes * 8;
        int crc_bits = 32;
        int L1D_reserved_len = required_total_bits - current_bits - crc_bits;

        if (L1D_reserved_len < 0) {
            L1D_reserved_len = 0;
        }

        if (L1D_reserved_len > 0) {
            if (config.preamble.L1D_version == 2) {
                frame_binarySequence += BinaryUtils::to_binary(config.preamble.L1D_bsid, 16);
                if (L1D_reserved_len > 16) {
                    std::string remaining_reserved(L1D_reserved_len - 16, '1');
                    frame_binarySequence += remaining_reserved;
                }
            } else {
                std::string L1D_reserved_bits(L1D_reserved_len, '1');
                frame_binarySequence += L1D_reserved_bits;
            }
        }

        std::string crc32 = BinaryUtils::calculate_crc32(frame_binarySequence);
        frame_binarySequence += crc32;

        std::string frame_hex = BinaryUtils::binary_string_to_hex(frame_binarySequence);

        int final_bytes = frame_hex.length() / 2;
        if (final_bytes != config.preamble.L1B_L1_Detail_size_bytes) {
            std::cerr << "WARNING Frame " << (frame_idx + 1) << ": Final L1D size (" << final_bytes
                      << " bytes) different than expected ("
                      << config.preamble.L1B_L1_Detail_size_bytes << " bytes)" << std::endl;
        }

        all_frames_binary += frame_binarySequence;

        all_frames_hex += "Frame #" + std::to_string(frame_idx + 1) + " (" + std::to_string(final_bytes) + " bytes)\n";
        all_frames_hex += frame_hex;
        if (frame_idx < config.frame_count - 1) {
            all_frames_hex += "\n";
        }
    }

    std::ofstream detailFile(config.detail_file_binary, std::ios::binary);
    if (detailFile.is_open()) {
        BinaryUtils::write_binary_to_file(detailFile, all_frames_binary);
        detailFile.close();
        std::cout << "\nL1-Detail multi-frame binary file saved: " << config.detail_file_binary << std::endl;
        std::cout << "Total frames: " << config.frame_count << std::endl;
        std::cout << "Total size: " << all_frames_binary.length() << " bits ("
                  << all_frames_binary.length() / 8 << " bytes)" << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save L1-Detail: " << config.detail_file_binary << std::endl;
    }

    std::ofstream detailFileHex(config.detail_file_hex);
    if (detailFileHex.is_open()) {
        detailFileHex << all_frames_hex;
        detailFileHex.close();
        std::cout << "Saved hexadecimal multi-frame L1-Detail: " << config.detail_file_hex << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save hexadecimal L1-Detail: " << config.detail_file_hex << std::endl;
    }
}
