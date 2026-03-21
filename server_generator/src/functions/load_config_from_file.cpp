#include "functions.h"

int extract_index_from_brackets(const std::string& str) {
    size_t start = str.find('[');
    size_t end = str.find(']');
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string index_str = str.substr(start + 1, end - start - 1);
        return std::stoi(index_str);
    }
    return -1;
}

void load_config_from_file() {
	std::ifstream file(CONFIG_FILE);

	if(!file.is_open()){
		std::cout << "Creating configuration:" << std::endl;
		std::cout << "Configuration file not found. Keeping default values..." << std::endl;
        save_config_to_file();
        std::cout << "Default configuration file created: " << CONFIG_FILE << std::endl;
		std::cout << "File created: " << CONFIG_FILE << std::endl;
	}
    std::cout << "Loading configuration from: " << CONFIG_FILE << std::endl;

    if (file.is_open()) {
        std::string line;
        int lines_processed = 0;
        int subframes_loaded = 0;
        int plps_loaded = 0;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                lines_processed++;

                std::cout << "Loading: " << key << " = " << value << std::endl;

                if (key == "major_version") {
                    bootstrap_info.major_version = std::stoi(value);
                }
                else if (key == "minor_version") bootstrap_info.minor_version = std::stoi(value);
                else if (key == "bootstrap_symbol") bootstrap_info.bootstrap_symbol = std::stoi(value);
                else if (key == "ea_wakeup") bootstrap_info.ea_wakeup = std::stoi(value);
                else if (key == "system_bandwidth") bootstrap_info.system_bandwidth = std::stoi(value);
                else if (key == "bsr_coefficient") bootstrap_info.bsr_coefficient = std::stoi(value);
                else if (key == "min_time_to_next") bootstrap_info.min_time_to_next = std::stoi(value);
                else if (key == "preamble_structure") bootstrap_info.preamble_structure = std::stoi(value);
                else if (key == "number_of_frames") global_frame_count = std::stoi(value);
                else if (key == "L1B_version") {
                    preamble.L1B_version = std::stoi(value);
                }
                else if (key == "L1B_mimo_scattered_pilot_encoding") preamble.L1B_mimo_scattered_pilot_encoding = std::stoi(value);
                else if (key == "L1B_lls_flag") preamble.L1B_lls_flag = std::stoi(value);
                else if (key == "L1B_time_info_flag") preamble.L1B_time_info_flag = std::stoi(value);
                else if (key == "L1B_return_channel_flag") preamble.L1B_return_channel_flag = std::stoi(value);
                else if (key == "L1B_papr_reduction") preamble.L1B_papr_reduction = std::stoi(value);
                else if (key == "L1B_frame_length_mode") preamble.L1B_frame_length_mode = std::stoi(value);
                else if (key == "L1B_frame_length") preamble.L1B_frame_length = std::stoi(value);
                else if (key == "L1B_excess_samples_per_symbol") preamble.L1B_excess_samples_per_symbol = std::stoi(value);
                else if (key == "L1B_time_offset") preamble.L1B_time_offset = std::stoi(value);
                else if (key == "L1B_additional_samples") preamble.L1B_additional_samples = std::stoi(value);
                else if (key == "L1B_num_subframes") {
                    preamble.L1B_num_subframes = std::stoi(value);
                    preamble.sub_frames.resize(preamble.L1B_num_subframes);
                }
                else if (key == "L1B_preamble_num_symbols") preamble.L1B_preamble_num_symbols = std::stoi(value);
                else if (key == "L1B_preamble_reduced_carriers") preamble.L1B_preamble_reduced_carriers = std::stoi(value);
                else if (key == "L1B_L1_Detail_content_tag") preamble.L1B_L1_Detail_content_tag = std::stoi(value);
                else if (key == "L1B_L1_Detail_size_bytes") preamble.L1B_L1_Detail_size_bytes = std::stoi(value);
                else if (key == "L1B_L1_Detail_fec_type") preamble.L1B_L1_Detail_fec_type = std::stoi(value);
                else if (key == "L1B_L1_Detail_additional_parity_mode") preamble.L1B_L1_Detail_additional_parity_mode = std::stoi(value);
                else if (key == "L1B_L1_Detail_total_cells") preamble.L1B_L1_Detail_total_cells = std::stoi(value);
                else if (key == "L1D_version") preamble.L1D_version = std::stoi(value);
                else if (key == "L1D_bsid") {
                    preamble.L1D_bsid = std::stoi(value);
                }
                else if (key == "L1D_num_rf") preamble.L1D_num_rf = std::stoi(value);
                else if (key == "L1D_rf_id") preamble.L1D_rf_id = std::stoi(value);
                else if (key == "L1D_bonded_bsid") preamble.L1D_bonded_bsid = std::stoi(value);
                else if (key == "L1D_time_sec") preamble.L1D_time_sec = std::stoi(value);
                else if (key == "L1D_time_msec") preamble.L1D_time_msec = std::stoi(value);
                else if (key == "L1D_time_usec") preamble.L1D_time_usec = std::stoi(value);
                else if (key == "L1D_time_nsec") preamble.L1D_time_nsec = std::stoi(value);

                else if (key.substr(0, 9) == "subframe.") {
                    std::string remaining = key.substr(9);
                    size_t first_dot = remaining.find('.');
                    if (first_dot != std::string::npos) {
                        int subframe_idx = extract_index_from_brackets(remaining.substr(0, first_dot));

                        if (subframe_idx == -1) {
                            std::cout << "Error: Invalid subframe index format: " << key << std::endl;
                            continue;
                        }
                        std::string rest = remaining.substr(first_dot + 1);
                        if (subframe_idx >= preamble.sub_frames.size()) {
                            preamble.sub_frames.resize(subframe_idx + 1);
                        }
                        if (rest.substr(0, 4) == "plp.") {
                            std::string plp_part = rest.substr(4);
                            size_t second_dot = plp_part.find('.');
                            if (second_dot != std::string::npos) {
                                int plp_idx = extract_index_from_brackets(plp_part.substr(0, second_dot));

                                if (plp_idx == -1) {
                                    std::cout << "Error: Invalid PLP index format: " << key << std::endl;
                                    continue;
                                }
                                std::string plp_property = plp_part.substr(second_dot + 1);

                                if (plp_idx >= preamble.sub_frames[subframe_idx].plps.size()) {
                                    preamble.sub_frames[subframe_idx].plps.resize(plp_idx + 1);
                                }
                                if (plp_property == "L1D_plp_id") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_id = std::stoi(value);
                                    plps_loaded++;
                                }
                                else if (plp_property == "L1D_plp_lls_flag") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_lls_flag = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_layer") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_layer = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_start") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_start = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_size") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_size = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_fec_type") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_fec_type = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_mod") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mod = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_cod") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_cod = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_TI_mode") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_TI_mode = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_TI_extended_interleaving") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_TI_extended_interleaving = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_CTI_depth") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_CTI_depth = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_mimo") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_mimo_stream_combining") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo_stream_combining = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_mimo_IQ_interleaving") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo_IQ_interleaving = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_mimo_PH") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo_PH = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_type") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_type = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_num_subslices") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_num_subslices = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_subslice_interval") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_subslice_interval = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_HTI_cell_interleaver") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_cell_interleaver = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_HTI_inter_subframe") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_inter_subframe = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_HTI_num_ti_blocks") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_num_ti_blocks = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_HTI_num_fec_blocks_max") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_num_fec_blocks_max = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_HTI_num_fec_blocks") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_num_fec_blocks = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_ldm_injection_level") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_ldm_injection_level = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_scrambler_type") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_scrambler_type = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_fec_block_start") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_fec_block_start = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_CTI_fec_block_start") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_CTI_fec_block_start = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_num_channel_bonded") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_num_channel_bonded = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_channel_bonding_format") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_channel_bonding_format = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_bonded_rf_id") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_bonded_rf_id = std::stoi(value);
                                }
                                else if (plp_property == "L1D_plp_CTI_start_row") {
                                    preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_CTI_start_row = std::stoi(value);
                                }
                            }
                        } else {
                            if (rest == "L1B_first_sub_mimo") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_mimo = std::stoi(value);
                                subframes_loaded++;
                            }
                            else if (rest == "L1B_first_sub_mimo_mixed") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_mimo_mixed = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_miso") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_miso = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_fft_size") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_fft_size = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_reduced_carriers") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_reduced_carriers = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_guard_interval") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_guard_interval = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_num_ofdm_symbols") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_num_ofdm_symbols = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_scattered_pilot_pattern") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_scattered_pilot_pattern = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_scattered_pilot_boost") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_scattered_pilot_boost = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_sbs_first") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_sbs_first = std::stoi(value);
                            }
                            else if (rest == "L1B_first_sub_sbs_last") {
                                preamble.sub_frames[subframe_idx].L1B_first_sub_sbs_last = std::stoi(value);
                            }
                            else if (rest == "L1D_mimo") {
                                preamble.sub_frames[subframe_idx].L1D_mimo = std::stoi(value);
                            }
                            else if (rest == "L1D_mimo_mixed") {
                                preamble.sub_frames[subframe_idx].L1D_mimo_mixed = std::stoi(value);
                            }
                            else if (rest == "L1D_miso") {
                                preamble.sub_frames[subframe_idx].L1D_miso = std::stoi(value);
                            }
                            else if (rest == "L1D_fft_size") {
                                preamble.sub_frames[subframe_idx].L1D_fft_size = std::stoi(value);
                            }
                            else if (rest == "L1D_reduced_carriers") {
                                preamble.sub_frames[subframe_idx].L1D_reduced_carriers = std::stoi(value);
                            }
                            else if (rest == "L1D_guard_interval") {
                                preamble.sub_frames[subframe_idx].L1D_guard_interval = std::stoi(value);
                            }
                            else if (rest == "L1D_num_ofdm_symbols") {
                                preamble.sub_frames[subframe_idx].L1D_num_ofdm_symbols = std::stoi(value);
                            }
                            else if (rest == "L1D_scattered_pilot_pattern") {
                                preamble.sub_frames[subframe_idx].L1D_scattered_pilot_pattern = std::stoi(value);
                            }
                            else if (rest == "L1D_scattered_pilot_boost") {
                                preamble.sub_frames[subframe_idx].L1D_scattered_pilot_boost = std::stoi(value);
                            }
                            else if (rest == "L1D_sbs_first") {
                                preamble.sub_frames[subframe_idx].L1D_sbs_first = std::stoi(value);
                            }
                            else if (rest == "L1D_sbs_last") {
                                preamble.sub_frames[subframe_idx].L1D_sbs_last = std::stoi(value);
                            }
                            else if (rest == "L1D_subframe_multiplex") {
                                preamble.sub_frames[subframe_idx].L1D_subframe_multiplex = std::stoi(value);
                            }
                            else if (rest == "L1D_frequency_interleaver") {
                                preamble.sub_frames[subframe_idx].L1D_frequency_interleaver = std::stoi(value);
                            }
                            else if (rest == "L1D_sbs_null_cells") {
                                preamble.sub_frames[subframe_idx].L1D_sbs_null_cells = std::stoi(value);
                            }
                            else if (rest == "L1D_num_plp") {
                                int num_plps = std::stoi(value);
                                if (num_plps > 0) {
                                    preamble.sub_frames[subframe_idx].plps.resize(num_plps);
                                }
                            }
                        }
                    }
                }
            }
        }

        file.close();
        std::cout << "Configuration loaded successfully!" << std::endl;
        std::cout << "Processed lines: " << lines_processed << std::endl;
        std::cout << "Loaded subframes: " << subframes_loaded << std::endl;
        std::cout << "Loaded PLPs: " << plps_loaded << std::endl;


        // Clamp preamble_num_symbols to valid range 0-3 (1-4 symbols)
        if (preamble.L1B_preamble_num_symbols < 0) {
            std::cout << "[LOAD] L1B_preamble_num_symbols adjusted from " << preamble.L1B_preamble_num_symbols << " to 0." << std::endl;
            preamble.L1B_preamble_num_symbols = 0;
        } else if (preamble.L1B_preamble_num_symbols > 3) {
            std::cout << "[LOAD] L1B_preamble_num_symbols adjusted from " << preamble.L1B_preamble_num_symbols << " to 3." << std::endl;
            preamble.L1B_preamble_num_symbols = 3;
        }
    }
}
