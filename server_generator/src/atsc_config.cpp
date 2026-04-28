#include "atsc_config.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <algorithm>

static int extract_index_from_brackets(const std::string& str) {
    size_t start = str.find('[');
    size_t end = str.find(']');
    if (start != std::string::npos && end != std::string::npos && end > start) {
        std::string index_str = str.substr(start + 1, end - start - 1);
        return std::stoi(index_str);
    }
    return -1;
}

ATSC_Config::ATSC_Config(const std::string& file)
    : config_file(file),
      basic_file_binary("config/basic.bin"),
      basic_file_hex("config/basic.hex"),
      detail_file_binary("config/detail.bin"),
      detail_file_hex("config/detail.hex"),
      frame_count(1)
{
    preamble.sub_frames.resize(1);
    preamble.sub_frames[0].plps.resize(1);
}

void ATSC_Config::load() {
	std::ifstream file(config_file);

	if(!file.is_open()){
		std::cout << "Creating configuration:" << std::endl;
		std::cout << "Configuration file not found. Keeping default values..." << std::endl;
        save();
        std::cout << "Default configuration file created: " << config_file << std::endl;
		std::cout << "File created: " << config_file << std::endl;
	}
    std::cout << "Loading configuration from: " << config_file << std::endl;

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
                    bootstrap.major_version = std::stoi(value);
                }
                else if (key == "minor_version") bootstrap.minor_version = std::stoi(value);
                else if (key == "bootstrap_symbol") bootstrap.bootstrap_symbol = std::stoi(value);
                else if (key == "ea_wakeup") bootstrap.ea_wakeup = std::stoi(value);
                else if (key == "system_bandwidth") bootstrap.system_bandwidth = std::stoi(value);
                else if (key == "bsr_coefficient") bootstrap.bsr_coefficient = std::stoi(value);
                else if (key == "min_time_to_next") bootstrap.min_time_to_next = std::stoi(value);
                else if (key == "preamble_structure") bootstrap.preamble_structure = std::stoi(value);
                else if (key == "number_of_frames") frame_count = std::stoi(value);
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
                        if (subframe_idx >= (int)preamble.sub_frames.size()) {
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

                                if (plp_idx >= (int)preamble.sub_frames[subframe_idx].plps.size()) {
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

        if (preamble.L1B_preamble_num_symbols < 0) {
            std::cout << "[LOAD] L1B_preamble_num_symbols adjusted from " << preamble.L1B_preamble_num_symbols << " to 0." << std::endl;
            preamble.L1B_preamble_num_symbols = 0;
        } else if (preamble.L1B_preamble_num_symbols > 3) {
            std::cout << "[LOAD] L1B_preamble_num_symbols adjusted from " << preamble.L1B_preamble_num_symbols << " to 3." << std::endl;
            preamble.L1B_preamble_num_symbols = 3;
        }
    }
}

void ATSC_Config::save() {

    if (preamble.L1B_preamble_num_symbols < 0) {
        preamble.L1B_preamble_num_symbols = 0;
    } else if (preamble.L1B_preamble_num_symbols > 3) {
        preamble.L1B_preamble_num_symbols = 3;
    }

    std::ofstream file(config_file);
    if (file.is_open()) {
        file << "# =============================================\n";
        file << "# ATSC 3.0 Modulator Configuration File\n";
        file << "# Auto-generated - Do not edit manually\n";
        file << "# =============================================\n\n";

        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
        file << "# Last updated: " << std::ctime(&currentTime) << "\n";

        file << "# =============================================\n";
        file << "# BOOTSTRAP CONFIGURATION\n";
        file << "# =============================================\n";
        file << "major_version=" << bootstrap.major_version << "\n";
        file << "minor_version=" << bootstrap.minor_version << "\n";
        file << "bootstrap_symbol=" << bootstrap.bootstrap_symbol << "\n";
        file << "ea_wakeup=" << bootstrap.ea_wakeup << "\n";
        file << "system_bandwidth=" << bootstrap.system_bandwidth << "\n";
        file << "bsr_coefficient=" << bootstrap.bsr_coefficient << "\n";
        file << "min_time_to_next=" << bootstrap.min_time_to_next << "\n";
        file << "preamble_structure=" << bootstrap.preamble_structure << "\n";
        file << "number_of_frames=" << frame_count << "\n";

        file << "# =============================================\n";
        file << "# PREAMBLE CONFIGURATION\n";
        file << "# =============================================\n";
        file << "L1B_version=" << preamble.L1B_version << "\n";
        file << "L1B_mimo_scattered_pilot_encoding=" << preamble.L1B_mimo_scattered_pilot_encoding << "\n";
        file << "L1B_lls_flag=" << (preamble.sub_frames.size() > 0 && preamble.sub_frames[0].plps.size() > 0 ? preamble.sub_frames[0].plps[0].L1D_plp_lls_flag : 0) << "\n";
        file << "L1B_time_info_flag=" << preamble.L1B_time_info_flag << "\n";
        file << "L1B_return_channel_flag=" << preamble.L1B_return_channel_flag << "\n";
        file << "L1B_papr_reduction=" << preamble.L1B_papr_reduction << "\n";
        file << "L1B_frame_length_mode=" << preamble.L1B_frame_length_mode << "\n";
        file << "L1B_frame_length=" << preamble.L1B_frame_length << "\n";
        file << "L1B_excess_samples_per_symbol=" << preamble.L1B_excess_samples_per_symbol << "\n";
        file << "L1B_time_offset=" << preamble.L1B_time_offset << "\n";
        file << "L1B_additional_samples=" << preamble.L1B_additional_samples << "\n";
        file << "L1B_num_subframes=" << preamble.L1B_num_subframes << "\n";
        file << "L1B_preamble_num_symbols=" << preamble.L1B_preamble_num_symbols << "\n";
        file << "L1B_preamble_reduced_carriers=" << preamble.L1B_preamble_reduced_carriers << "\n";
        file << "L1B_L1_Detail_content_tag=" << preamble.L1B_L1_Detail_content_tag << "\n";
        if(preamble.L1B_L1_Detail_size_bytes < 25){
        	preamble.L1B_L1_Detail_size_bytes = 25;
        }
        file << "L1B_L1_Detail_size_bytes=" << preamble.L1B_L1_Detail_size_bytes << "\n";
        file << "L1B_L1_Detail_fec_type=" << preamble.L1B_L1_Detail_fec_type << "\n";
        file << "L1B_L1_Detail_additional_parity_mode=" << preamble.L1B_L1_Detail_additional_parity_mode << "\n";
        file << "L1B_L1_Detail_total_cells=" << preamble.L1B_L1_Detail_total_cells << "\n";
        file << "L1D_version=" << preamble.L1D_version << "\n";
        file << "L1D_bsid=" << preamble.L1D_bsid << "\n";
        file << "L1D_num_rf=" << preamble.L1D_num_rf << "\n";
        file << "L1D_rf_id=" << preamble.L1D_rf_id << "\n";
        file << "L1D_bonded_bsid=" << preamble.L1D_bonded_bsid << "\n";
        file << "L1D_time_sec=" << preamble.L1D_time_sec << "\n";
        file << "L1D_time_msec=" << preamble.L1D_time_msec << "\n";
        file << "L1D_time_usec=" << preamble.L1D_time_usec << "\n";
        file << "L1D_time_nsec=" << preamble.L1D_time_nsec << "\n";

        if (preamble.sub_frames.size() > 0) {
            file << "# =============================================\n";
            file << "# SUBFRAMES & PLPs CONFIGURATION\n";
            file << "# =============================================\n";

            for (size_t i = 0; i < preamble.sub_frames.size(); i++) {
                if (i == 0) {
                    file << "\n# --- Subframe Basic " << i << " ---\n";
                    file << "subframe.[" << i << "].L1B_first_sub_mimo=" << preamble.sub_frames[i].L1B_first_sub_mimo << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_mimo_mixed=" << preamble.sub_frames[i].L1B_first_sub_mimo_mixed << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_miso=" << preamble.sub_frames[i].L1B_first_sub_miso << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_fft_size=" << preamble.sub_frames[i].L1B_first_sub_fft_size << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_reduced_carriers=" << preamble.sub_frames[i].L1B_first_sub_reduced_carriers << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_guard_interval=" << preamble.sub_frames[i].L1B_first_sub_guard_interval << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_num_ofdm_symbols=" << preamble.sub_frames[i].L1B_first_sub_num_ofdm_symbols << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_scattered_pilot_pattern=" << preamble.sub_frames[i].L1B_first_sub_scattered_pilot_pattern << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_scattered_pilot_boost=" << preamble.sub_frames[i].L1B_first_sub_scattered_pilot_boost << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_sbs_first=" << preamble.sub_frames[i].L1B_first_sub_sbs_first << "\n";
                    file << "subframe.[" << i << "].L1B_first_sub_sbs_last=" << preamble.sub_frames[i].L1B_first_sub_sbs_last << "\n";
                } else {
                    file << "\n# --- Subframe Detail " << i << " ---\n";
                    file << "subframe.[" << i << "].L1D_mimo=" << preamble.sub_frames[i].L1D_mimo << "\n";
                    file << "subframe.[" << i << "].L1D_mimo_mixed=" << preamble.sub_frames[i].L1D_mimo_mixed << "\n";
                    file << "subframe.[" << i << "].L1D_miso=" << preamble.sub_frames[i].L1D_miso << "\n";
                    file << "subframe.[" << i << "].L1D_fft_size=" << preamble.sub_frames[i].L1D_fft_size << "\n";
                    file << "subframe.[" << i << "].L1D_reduced_carriers=" << preamble.sub_frames[i].L1D_reduced_carriers << "\n";
                    file << "subframe.[" << i << "].L1D_guard_interval=" << preamble.sub_frames[i].L1D_guard_interval << "\n";
                    file << "subframe.[" << i << "].L1D_num_ofdm_symbols=" << preamble.sub_frames[i].L1D_num_ofdm_symbols << "\n";
                    file << "subframe.[" << i << "].L1D_scattered_pilot_pattern=" << preamble.sub_frames[i].L1D_scattered_pilot_pattern << "\n";
                    file << "subframe.[" << i << "].L1D_scattered_pilot_boost=" << preamble.sub_frames[i].L1D_scattered_pilot_boost << "\n";
                    file << "subframe.[" << i << "].L1D_sbs_first=" << preamble.sub_frames[i].L1D_sbs_first << "\n";
                    file << "subframe.[" << i << "].L1D_sbs_last=" << preamble.sub_frames[i].L1D_sbs_last << "\n";
                }

                file << "subframe.[" << i << "].L1D_subframe_multiplex=" << preamble.sub_frames[i].L1D_subframe_multiplex << "\n";
                file << "subframe.[" << i << "].L1D_frequency_interleaver=" << preamble.sub_frames[i].L1D_frequency_interleaver << "\n";
                file << "subframe.[" << i << "].L1D_sbs_null_cells=" << preamble.sub_frames[i].L1D_sbs_null_cells << "\n";
                file << "subframe.[" << i << "].L1D_num_plp=" << preamble.sub_frames[i].plps.size() << "\n";

                if (preamble.sub_frames[i].plps.size() > 0) {
                    for (size_t j = 0; j < preamble.sub_frames[i].plps.size(); j++) {
                        file << "\n# Subframe " << i << " - PLP " << j << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_id=" << preamble.sub_frames[i].plps[j].L1D_plp_id << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_lls_flag=" << preamble.sub_frames[i].plps[j].L1D_plp_lls_flag << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_layer=" << preamble.sub_frames[i].plps[j].L1D_plp_layer << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_start=" << preamble.sub_frames[i].plps[j].L1D_plp_start << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_size=" << preamble.sub_frames[i].plps[j].L1D_plp_size << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_scrambler_type=" << preamble.sub_frames[i].plps[j].L1D_plp_scrambler_type << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_fec_type=" << preamble.sub_frames[i].plps[j].L1D_plp_fec_type << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mod=" << preamble.sub_frames[i].plps[j].L1D_plp_mod << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_cod=" << preamble.sub_frames[i].plps[j].L1D_plp_cod << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_TI_mode=" << preamble.sub_frames[i].plps[j].L1D_plp_TI_mode << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_fec_block_start=" << preamble.sub_frames[i].plps[j].L1D_plp_fec_block_start << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_CTI_fec_block_start=" << preamble.sub_frames[i].plps[j].L1D_plp_CTI_fec_block_start << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_num_channel_bonded=" << preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_channel_bonding_format=" << preamble.sub_frames[i].plps[j].L1D_plp_channel_bonding_format << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_bonded_rf_id=" << preamble.sub_frames[i].plps[j].L1D_plp_bonded_rf_id << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_TI_extended_interleaving=" << preamble.sub_frames[i].plps[j].L1D_plp_TI_extended_interleaving << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_CTI_depth=" << preamble.sub_frames[i].plps[j].L1D_plp_CTI_depth << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_CTI_start_row=" << preamble.sub_frames[i].plps[j].L1D_plp_CTI_start_row << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo=" << preamble.sub_frames[i].plps[j].L1D_plp_mimo << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo_stream_combining=" << preamble.sub_frames[i].plps[j].L1D_plp_mimo_stream_combining << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo_IQ_interleaving=" << preamble.sub_frames[i].plps[j].L1D_plp_mimo_IQ_interleaving << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo_PH=" << preamble.sub_frames[i].plps[j].L1D_plp_mimo_PH << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_type=" << preamble.sub_frames[i].plps[j].L1D_plp_type << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_num_subslices=" << preamble.sub_frames[i].plps[j].L1D_plp_num_subslices << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_subslice_interval=" << preamble.sub_frames[i].plps[j].L1D_plp_subslice_interval << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_inter_subframe=" << preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_num_ti_blocks=" << preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_ti_blocks << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_num_fec_blocks_max=" << preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks_max << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_num_fec_blocks=" << preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_cell_interleaver=" << preamble.sub_frames[i].plps[j].L1D_plp_HTI_cell_interleaver << "\n";
                        file << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_ldm_injection_level=" << preamble.sub_frames[i].plps[j].L1D_plp_ldm_injection_level << "\n";
                    }
                }
            }
        }

        file << "\n# =============================================\n";
        file << "# END OF CONFIGURATION FILE\n";
        file << "# =============================================\n";

        file.close();
        std::cout << "Full configuration saved in: " << config_file << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save configuration file. " << config_file << std::endl;
    }
}

std::string ATSC_Config::process_command(const std::string& command) {
    std::string reply = "";
    std::string cmd = command;

    std::cout << "Processing: " << cmd << std::endl;

    cmd.erase(0, cmd.find_first_not_of(" \t\r\n"));
    cmd.erase(cmd.find_last_not_of(" \t\r\n") + 1);

    if (cmd.substr(0, 16) == "SET_FRAME_COUNT=") {
            frame_count = std::stoi(cmd.substr(16));
            reply = "SET_FRAME_COUNT Done\n";
            std::cout << "Frame Count = " << frame_count << std::endl;
        }

    if (cmd.substr(0, 10) == "GET_CONFIG") {
        std::cout << "Sending current configuration" << std::endl;
        reply += std::to_string(bootstrap.major_version) + "\n";
        reply += std::to_string(bootstrap.minor_version) + "\n";
        reply += std::to_string(bootstrap.bootstrap_symbol) + "\n";
        reply += std::to_string(bootstrap.ea_wakeup) + "\n";
        reply += std::to_string(bootstrap.system_bandwidth) + "\n";
        reply += std::to_string(bootstrap.bsr_coefficient) + "\n";
        reply += std::to_string(bootstrap.min_time_to_next) + "\n";
        reply += std::to_string(bootstrap.preamble_structure) + "\n";
        reply += std::to_string(frame_count) + "\n";

        reply += std::to_string(preamble.L1B_version) + "\n";
        reply += std::to_string(preamble.L1B_mimo_scattered_pilot_encoding) + "\n";

        reply += std::to_string(preamble.L1B_L1_Detail_fec_type) + "\n";
        reply += std::to_string(preamble.L1B_time_info_flag) + "\n";
        reply += std::to_string(preamble.L1B_frame_length_mode) + "\n";
        reply += std::to_string(preamble.L1B_frame_length) + "\n";
        reply += std::to_string(preamble.L1B_num_subframes) + "\n";

        reply += std::to_string(preamble.L1D_version) + "\n";
        reply += std::to_string(preamble.L1D_bsid) + "\n";

        reply += "SUBFRAMES_START\n";

        for (size_t i = 0; i < preamble.sub_frames.size(); i++) {
            if (i == 0){
                reply += "SUBFRAME Basic:" + std::to_string(i) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_mimo) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_miso) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_fft_size) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_reduced_carriers) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_guard_interval) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_num_ofdm_symbols) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_scattered_pilot_pattern) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_scattered_pilot_boost) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_sbs_first) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_sbs_last) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1B_first_sub_mimo_mixed) + "\n";

            }else {
                reply += "SUBFRAME Detail:" + std::to_string(i) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_mimo) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_miso) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_fft_size) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_reduced_carriers) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_guard_interval) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_num_ofdm_symbols) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_scattered_pilot_pattern) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_scattered_pilot_boost) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_sbs_first) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_sbs_last) + "\n";
                reply += std::to_string(preamble.sub_frames[i].L1D_mimo_mixed) + "\n";

            }
            reply += std::to_string(preamble.sub_frames[i].L1D_frequency_interleaver) + "\n";
            reply += std::to_string(preamble.sub_frames[i].plps.size()) + "\n";
            for (size_t j = 0; j < preamble.sub_frames[i].plps.size(); j++) {
                reply += "PLP:" + std::to_string(j) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_id) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_lls_flag) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_layer) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_start) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_size) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_fec_type) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_mod) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_cod) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_TI_mode) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_TI_extended_interleaving) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_CTI_depth) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_mimo) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_mimo_stream_combining) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_mimo_IQ_interleaving) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_mimo_PH) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_type) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_num_subslices) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_subslice_interval) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_HTI_cell_interleaver) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_ti_blocks) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks_max) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks) + "\n";
                reply += std::to_string(preamble.sub_frames[i].plps[j].L1D_plp_ldm_injection_level) + "\n";
            }
        }

        reply += "CONFIG_END\n";
    }
    else if (cmd.substr(0, 18) == "SET_MAJOR_VERSION=") {
        bootstrap.major_version = std::stoi(cmd.substr(18));
        reply = "SET_MAJOR_VERSION Done\n";
        std::cout << "Major Version = " << bootstrap.major_version << std::endl;
    }
    else if (cmd.substr(0, 18) == "SET_MINOR_VERSION=") {
        bootstrap.minor_version = std::stoi(cmd.substr(18));
        reply = "SET_MINOR_VERSION Done\n";
        std::cout << "Minor Version = " << bootstrap.minor_version << std::endl;
    }
    else if (cmd.substr(0, 21) == "SET_BOOTSTRAP_SYMBOL=") {
        bootstrap.bootstrap_symbol = std::stoi(cmd.substr(21));
        reply = "SET_BOOTSTRAP_SYMBOL Done\n";
        std::cout << "Bootstrap Symbol = " << bootstrap.bootstrap_symbol << std::endl;
    }
    else if (cmd.substr(0, 14) == "SET_EA_WAKEUP=") {
        bootstrap.ea_wakeup = std::stoi(cmd.substr(14));
        reply = "SET_EA_WAKEUP Done\n";
        std::cout << "EA Wakeup = " << bootstrap.ea_wakeup << std::endl;
    }
    else if (cmd.substr(0, 21) == "SET_SYSTEM_BANDWIDTH=") {
        bootstrap.system_bandwidth = std::stoi(cmd.substr(21));
        reply = "SET_SYSTEM_BANDWIDTH Done\n";
        std::cout << "System Bandwidth = " << bootstrap.system_bandwidth << std::endl;
    }
    else if (cmd.substr(0, 20) == "SET_BSR_COEFFICIENT=") {
        bootstrap.bsr_coefficient = std::stoi(cmd.substr(20));
        reply = "SET_BSR_COEFFICIENT Done\n";
        std::cout << "BSR Coefficient = " << bootstrap.bsr_coefficient << std::endl;
    }
    else if (cmd.substr(0, 21) == "SET_MIN_TIME_TO_NEXT=") {
        bootstrap.min_time_to_next = std::stoi(cmd.substr(21));
        reply = "SET_MIN_TIME_TO_NEXT Done\n";
        std::cout << "Min Time To Next = " << bootstrap.min_time_to_next << std::endl;
    }
    else if (cmd.substr(0, 23) == "SET_PREAMBLE_STRUCTURE=") {
        bootstrap.preamble_structure = std::stoi(cmd.substr(23));
        reply = "SET_PREAMBLE_STRUCTURE Done\n";
        std::cout << "Preamble Structure = " << bootstrap.preamble_structure << std::endl;
    }
    else if (cmd.substr(0, 16) == "SET_L1B_VERSION=") {
        preamble.L1B_version = std::stoi(cmd.substr(16));
        reply = "SET_L1B_VERSION Done\n";
        std::cout << "L1B Version = " << preamble.L1B_version << std::endl;
    }
    else if (cmd.substr(0, 39) == "SET_L1B_MIMO_SCATTERRED_PILOT_ENCODING=") {
        preamble.L1B_mimo_scattered_pilot_encoding = std::stoi(cmd.substr(39));
        reply = "SET_L1B_MIMO_SCATTERRED_PILOT_ENCODING Done\n";
        std::cout << "L1B MIMO Encoding = " << preamble.L1B_mimo_scattered_pilot_encoding << std::endl;
    }
    else if (cmd.substr(0, 20) == "SET_DETAIL_FEC_TYPE=") {
        preamble.L1B_L1_Detail_fec_type = std::stoi(cmd.substr(20));
        reply = "SET_DETAIL_FEC_TYPE Done\n";
        std::cout << "Detail FEC Type = " << preamble.L1B_L1_Detail_fec_type << std::endl;
    }
    else if (cmd.substr(0, 19) == "SET_TIME_INFO_FLAG=") {
        preamble.L1B_time_info_flag = std::stoi(cmd.substr(19));
        reply = "SET_TIME_INFO_FLAG Done\n";
        std::cout << "Time Info Flag = " << preamble.L1B_time_info_flag << std::endl;
    }
    else if (cmd.substr(0, 22) == "SET_FRAME_LENGHT_MODE=") {
        preamble.L1B_frame_length_mode = std::stoi(cmd.substr(22));
        reply = "SET_FRAME_LENGHT_MODE Done\n";
        std::cout << "Frame Length Mode = " << preamble.L1B_frame_length_mode << std::endl;
    }
    else if (cmd.substr(0, 17) == "SET_FRAME_LENGHT=") {
        preamble.L1B_frame_length = std::stoi(cmd.substr(17));
        reply = "SET_FRAME_LENGHT Done\n";
        std::cout << "Frame Length = " << preamble.L1B_frame_length << std::endl;
    }
    else if (cmd.substr(0, 24) == "SET_NUMBER_OF_SUBFRAMES=") {
        int new_count = std::stoi(cmd.substr(24));
        preamble.L1B_num_subframes = new_count;
        preamble.sub_frames.resize(new_count);
        reply = "SET_NUMBER_OF_SUBFRAMES Done\n";
        std::cout << "Number of Subframes = " << preamble.L1B_num_subframes << std::endl;
    }
    else if (cmd.substr(0, 25) == "SET_PREAMBLE_NUM_SYMBOLS=") {
        preamble.L1B_preamble_num_symbols = std::stoi(cmd.substr(25));
        reply = "SET_PREAMBLE_NUM_SYMBOLS Done\n";
        std::cout << "Preamble Num Symbols = " << preamble.L1B_preamble_num_symbols << std::endl;
    }
    else if (cmd.substr(0, 16) == "SET_L1D_VERSION=") {
        preamble.L1D_version = std::stoi(cmd.substr(16));
        reply = "SET_L1D_VERSION Done\n";
        std::cout << "L1D Version = " << preamble.L1D_version << std::endl;
    }
    else if (cmd.substr(0, 13) == "SET_L1D_BSID=") {
        int bsid_value = std::stoi(cmd.substr(13));
        preamble.L1D_bsid = bsid_value;
        reply = "SET_L1D_BSID Done\n";
        std::cout << "L1D BSID = " << preamble.L1D_bsid << std::endl;
    }
    else if (cmd.substr(0, 13) == "SET_SUBFRAME_") {
        size_t idx_pos = cmd.find('_', 13);
        if (idx_pos != std::string::npos) {
            int subframe_idx = std::stoi(cmd.substr(13, idx_pos - 13));
            std::string param_part = cmd.substr(idx_pos + 1);

            std::cout << "Subframe " << subframe_idx << " - " << param_part << std::endl;

            if (subframe_idx >= (int)preamble.sub_frames.size()) {
                preamble.sub_frames.resize(subframe_idx + 1);
                std::cout << "Redimensionado para " << (subframe_idx + 1) << " subframes" << std::endl;
            }

            if (param_part.substr(0, 19) == "L1B_first_sub_mimo=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_mimo = std::stoi(param_part.substr(19));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_mimo Done\n";
                std::cout << "L1B First Sub MIMO = " << preamble.sub_frames[subframe_idx].L1B_first_sub_mimo << std::endl;
            }
            else if (param_part.substr(0, 19) == "L1B_first_sub_miso=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_miso = std::stoi(param_part.substr(19));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_miso Done\n";
                std::cout << "L1B First Sub MISO = " << preamble.sub_frames[subframe_idx].L1B_first_sub_miso << std::endl;
            }
            else if (param_part.substr(0, 25) == "L1B_first_sub_mimo_mixed=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_mimo_mixed = std::stoi(param_part.substr(25));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_mimo_mixed Done\n";
                std::cout << "L1B First Sub MIMO Mixed = " << preamble.sub_frames[subframe_idx].L1B_first_sub_mimo_mixed << std::endl;
            }
            else if (param_part.substr(0, 23) == "L1B_first_sub_fft_size=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_fft_size = std::stoi(param_part.substr(23));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_fft_size Done\n";
                std::cout << "L1B First Sub FFT Size = " << preamble.sub_frames[subframe_idx].L1B_first_sub_fft_size << std::endl;
            }
            else if (param_part.substr(0, 31) == "L1B_first_sub_reduced_carriers=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_reduced_carriers = std::stoi(param_part.substr(31));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_reduced_carriers Done\n";
                std::cout << "L1B First Sub Reduced Carriers = " << preamble.sub_frames[subframe_idx].L1B_first_sub_reduced_carriers << std::endl;
            }
            else if (param_part.substr(0, 29) == "L1B_first_sub_guard_interval=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_guard_interval = std::stoi(param_part.substr(29));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_guard_interval Done\n";
                std::cout << "L1B First Sub Guard Interval = " << preamble.sub_frames[subframe_idx].L1B_first_sub_guard_interval << std::endl;
            }
            else if (param_part.substr(0, 31) == "L1B_first_sub_num_ofdm_symbols=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_num_ofdm_symbols = std::stoi(param_part.substr(31));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_num_ofdm_symbols Done\n";
                std::cout << "L1B First Sub Num OFDM Symbols = " << preamble.sub_frames[subframe_idx].L1B_first_sub_num_ofdm_symbols << std::endl;
            }
            else if (param_part.substr(0, 38) == "L1B_first_sub_scattered_pilot_pattern=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_scattered_pilot_pattern = std::stoi(param_part.substr(38));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_scattered_pilot_pattern Done\n";
                std::cout << "L1B First Sub Scattered Pilot Pattern = " << preamble.sub_frames[subframe_idx].L1B_first_sub_scattered_pilot_pattern << std::endl;
            }
            else if (param_part.substr(0, 36) == "L1B_first_sub_scattered_pilot_boost=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_scattered_pilot_boost = std::stoi(param_part.substr(36));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_scattered_pilot_boost Done\n";
                std::cout << "L1B First Sub Scattered Pilot Boost = " << preamble.sub_frames[subframe_idx].L1B_first_sub_scattered_pilot_boost << std::endl;
            }
            else if (param_part.substr(0, 24) == "L1B_first_sub_sbs_first=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_sbs_first = std::stoi(param_part.substr(24));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_sbs_first Done\n";
                std::cout << "L1B First Sub SBS First = " << preamble.sub_frames[subframe_idx].L1B_first_sub_sbs_first << std::endl;
            }
            else if (param_part.substr(0, 23) == "L1B_first_sub_sbs_last=") {
                preamble.sub_frames[subframe_idx].L1B_first_sub_sbs_last = std::stoi(param_part.substr(23));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1B_first_sub_sbs_last Done\n";
                std::cout << "L1B First Sub SBS Last = " << preamble.sub_frames[subframe_idx].L1B_first_sub_sbs_last << std::endl;
            }
            else if (param_part.substr(0, 10) == "L1D_mimo=") {
                preamble.sub_frames[subframe_idx].L1D_mimo = std::stoi(param_part.substr(10));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_mimo Done\n";
                std::cout << "L1D MIMO = " << preamble.sub_frames[subframe_idx].L1D_mimo << std::endl;
            }
            else if (param_part.substr(0, 16) == "L1D_mimo_mixed=") {
                preamble.sub_frames[subframe_idx].L1D_mimo_mixed = std::stoi(param_part.substr(16));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_mimo_mixed Done\n";
                std::cout << "L1D MIMO Mixed = " << preamble.sub_frames[subframe_idx].L1D_mimo_mixed << std::endl;
            }
            else if (param_part.substr(0, 10) == "L1D_miso=") {
                preamble.sub_frames[subframe_idx].L1D_miso = std::stoi(param_part.substr(10));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_miso Done\n";
                std::cout << "L1D MISO = " << preamble.sub_frames[subframe_idx].L1D_miso << std::endl;
            }
            else if (param_part.substr(0, 13) == "L1D_fft_size=") {
                preamble.sub_frames[subframe_idx].L1D_fft_size = std::stoi(param_part.substr(13));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_fft_size Done\n";
                std::cout << "L1D FFT Size = " << preamble.sub_frames[subframe_idx].L1D_fft_size << std::endl;
            }
            else if (param_part.substr(0, 21) == "L1D_reduced_carriers=") {
                preamble.sub_frames[subframe_idx].L1D_reduced_carriers = std::stoi(param_part.substr(21));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_reduced_carriers Done\n";
                std::cout << "L1D Reduced Carriers = " << preamble.sub_frames[subframe_idx].L1D_reduced_carriers << std::endl;
            }
            else if (param_part.substr(0, 19) == "L1D_guard_interval=") {
                preamble.sub_frames[subframe_idx].L1D_guard_interval = std::stoi(param_part.substr(19));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_guard_interval Done\n";
                std::cout << "L1D Guard Interval = " << preamble.sub_frames[subframe_idx].L1D_guard_interval << std::endl;
            }
            else if (param_part.substr(0, 21) == "L1D_num_ofdm_symbols=") {
                preamble.sub_frames[subframe_idx].L1D_num_ofdm_symbols = std::stoi(param_part.substr(21));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_num_ofdm_symbols Done\n";
                std::cout << "L1D Num OFDM Symbols = " << preamble.sub_frames[subframe_idx].L1D_num_ofdm_symbols << std::endl;
            }
            else if (param_part.substr(0, 28) == "L1D_scattered_pilot_pattern=") {
                preamble.sub_frames[subframe_idx].L1D_scattered_pilot_pattern = std::stoi(param_part.substr(28));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_scattered_pilot_pattern Done\n";
                std::cout << "L1D Scattered Pilot Pattern = " << preamble.sub_frames[subframe_idx].L1D_scattered_pilot_pattern << std::endl;
            }
            else if (param_part.substr(0, 26) == "L1D_scattered_pilot_boost=") {
                preamble.sub_frames[subframe_idx].L1D_scattered_pilot_boost = std::stoi(param_part.substr(26));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_scattered_pilot_boost Done\n";
                std::cout << "L1D Scattered Pilot Boost = " << preamble.sub_frames[subframe_idx].L1D_scattered_pilot_boost << std::endl;
            }
            else if (param_part.substr(0, 14) == "L1D_sbs_first=") {
                preamble.sub_frames[subframe_idx].L1D_sbs_first = std::stoi(param_part.substr(14));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_sbs_first Done\n";
                std::cout << "L1D SBS First = " << preamble.sub_frames[subframe_idx].L1D_sbs_first << std::endl;
            }
            else if (param_part.substr(0, 13) == "L1D_sbs_last=") {
                preamble.sub_frames[subframe_idx].L1D_sbs_last = std::stoi(param_part.substr(13));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_sbs_last Done\n";
                std::cout << "L1D SBS Last = " << preamble.sub_frames[subframe_idx].L1D_sbs_last << std::endl;
            }
            else if (param_part.substr(0, 26) == "L1D_frequency_interleaver=") {
                preamble.sub_frames[subframe_idx].L1D_frequency_interleaver = std::stoi(param_part.substr(26));
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_L1D_frequency_interleaver Done\n";
                std::cout << "L1D Frequency Interleaver = " << preamble.sub_frames[subframe_idx].L1D_frequency_interleaver << std::endl;
            }
            else if (param_part.substr(0, 10) == "PLP_COUNT=") {
                int plp_count = std::stoi(param_part.substr(10));
                preamble.sub_frames[subframe_idx].plps.resize(plp_count);
                reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_COUNT Done\n";
                std::cout << "PLPs resized to " << plp_count << " elements" << std::endl;
            }
            else if (param_part.substr(0, 4) == "PLP_") {
                size_t plp_idx_pos = param_part.find('_', 4);
                if (plp_idx_pos != std::string::npos) {
                    int plp_idx = std::stoi(param_part.substr(4, plp_idx_pos - 4));
                    std::string plp_param = param_part.substr(plp_idx_pos + 1);

                    if (plp_idx >= (int)preamble.sub_frames[subframe_idx].plps.size()) {
                        preamble.sub_frames[subframe_idx].plps.resize(plp_idx + 1);
                        std::cout << "PLPs resized to " << (plp_idx + 1) << " elements" << std::endl;
                    }

                    std::cout << "PLP " << plp_idx << " - " << plp_param << std::endl;

                    if (plp_param.substr(0, 3) == "ID=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_id = std::stoi(plp_param.substr(3));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_ID Done\n";
                    }
                    else if (plp_param.substr(0, 9) == "LLS_FLAG=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_lls_flag = std::stoi(plp_param.substr(9));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_LLS_FLAG Done\n";
                    }
                    else if (plp_param.substr(0, 6) == "LAYER=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_layer = std::stoi(plp_param.substr(6));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_LAYER Done\n";
                    }
                    else if (plp_param.substr(0, 6) == "START=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_start = std::stoi(plp_param.substr(6));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_START Done\n";
                    }
                    else if (plp_param.substr(0, 5) == "SIZE=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_size = std::stoi(plp_param.substr(5));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_SIZE Done\n";
                    }
                    else if (plp_param.substr(0, 9) == "FEC_TYPE=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_fec_type = std::stoi(plp_param.substr(9));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_FEC_TYPE Done\n";
                    }
                    else if (plp_param.substr(0, 10) == "MOD_ORDER=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mod = std::stoi(plp_param.substr(10));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_MOD_ORDER Done\n";
                    }
                    else if (plp_param.substr(0, 10) == "CODE_RATE=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_cod = std::stoi(plp_param.substr(10));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_CODE_RATE Done\n";
                    }
                    else if (plp_param.substr(0, 8) == "TI_MODE=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_TI_mode = std::stoi(plp_param.substr(8));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_TI_MODE Done\n";
                    }
                    else if (plp_param.substr(0, 12) == "TI_EXTENDED=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_TI_extended_interleaving = std::stoi(plp_param.substr(12));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_TI_EXTENDED Done\n";
                    }
                    else if (plp_param.substr(0, 10) == "CTI_DEPTH=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_CTI_depth = std::stoi(plp_param.substr(10));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_CTI_DEPTH Done\n";
                    }
                    else if (plp_param.substr(0, 9) == "MIMO_PLP=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo = std::stoi(plp_param.substr(9));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_MIMO_PLP Done\n";
                    }
                    else if (plp_param.substr(0, 17) == "STREAM_COMBINING=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo_stream_combining = std::stoi(plp_param.substr(17));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_STREAM_COMBINING Done\n";
                    }
                    else if (plp_param.substr(0, 15) == "IQ_INTERVALING=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo_IQ_interleaving = std::stoi(plp_param.substr(15));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_IQ_INTERVALING Done\n";
                    }
                    else if (plp_param.substr(0, 14) == "PHASE_HOPPING=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mimo_PH = std::stoi(plp_param.substr(14));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_PHASE_HOPPING Done\n";
                    }
                    else if (plp_param.substr(0, 5) == "TYPE=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_type = std::stoi(plp_param.substr(5));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_TYPE Done\n";
                    }
                    else if (plp_param.substr(0, 13) == "NUM_SUBSLICE=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_num_subslices = std::stoi(plp_param.substr(13));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_NUM_SUBSLICE Done\n";
                    }
                    else if (plp_param.substr(0, 18) == "SUBSLICE_INTERVAL=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_subslice_interval = std::stoi(plp_param.substr(18));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_SUBSLICE_INTERVAL Done\n";
                    }
                    else if (plp_param.substr(0, 16) == "CELL_INTERVALER=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_cell_interleaver = std::stoi(plp_param.substr(16));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_CELL_INTERVALER Done\n";
                    }
                    else if (plp_param.substr(0, 15) == "INTER_SUBFRAME=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_inter_subframe = std::stoi(plp_param.substr(15));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_INTER_SUBFRAME Done\n";
                    }
                    else if (plp_param.substr(0, 14) == "NUM_TI_BLOCKS=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_num_ti_blocks = std::stoi(plp_param.substr(14));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_NUM_TI_BLOCKS Done\n";
                    }
                    else if (plp_param.substr(0, 19) == "NUM_FEC_BLOCKS_MAX=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_num_fec_blocks_max = std::stoi(plp_param.substr(19));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_NUM_FEC_BLOCKS_MAX Done\n";
                    }
                    else if (plp_param.substr(0, 15) == "NUM_FEC_BLOCKS=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_HTI_num_fec_blocks = std::stoi(plp_param.substr(15));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_NUM_FEC_BLOCKS Done\n";
                    }
                    else if (plp_param.substr(0, 20) == "LDM_INJECTION_LEVEL=") {
                        preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_ldm_injection_level = std::stoi(plp_param.substr(20));
                        reply = "SET_SUBFRAME_" + std::to_string(subframe_idx) + "_PLP_" + std::to_string(plp_idx) + "_LDM_INJECTION_LEVEL Done\n";
                    }
                    else {
                        reply = "ERROR: Unknown PLP parameter: " + plp_param + "\n";
                        std::cout << "Unknown PLP parameter: " << plp_param << std::endl;
                    }
                }
            }
            else {
                reply = "ERRO: Unknown subframe parameter: " + param_part + "\n";
                std::cout << "Unknown subframe parameter: " << param_part << std::endl;
            }
        }
    }
    else {
        reply = "ERRO: Unknown command: " + cmd + "\n";
        std::cout << "Unknown command: " << cmd << std::endl;
    }

    return reply;
}
