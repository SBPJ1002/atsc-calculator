#include "functions.h"

void save_config_to_file() {
    // Clamp preamble_num_symbols to valid range 0-3 (1-4 symbols)
    if (preamble.L1B_preamble_num_symbols < 0) {
        preamble.L1B_preamble_num_symbols = 0;
    } else if (preamble.L1B_preamble_num_symbols > 3) {
        preamble.L1B_preamble_num_symbols = 3;
    }

    std::ofstream file(CONFIG_FILE);
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
        file << "major_version=" << bootstrap_info.major_version << "\n";
        file << "minor_version=" << bootstrap_info.minor_version << "\n";
        file << "bootstrap_symbol=" << bootstrap_info.bootstrap_symbol << "\n";
        file << "ea_wakeup=" << bootstrap_info.ea_wakeup << "\n";
        file << "system_bandwidth=" << bootstrap_info.system_bandwidth << "\n";
        file << "bsr_coefficient=" << bootstrap_info.bsr_coefficient << "\n";
        file << "min_time_to_next=" << bootstrap_info.min_time_to_next << "\n";
        file << "preamble_structure=" << bootstrap_info.preamble_structure << "\n";
        file << "number_of_frames=" << global_frame_count << "\n";

        file << "# =============================================\n";
        file << "# PREAMBLE CONFIGURATION\n";
        file << "# =============================================\n";
        file << "L1B_version=" << preamble.L1B_version << "\n";
        file << "L1B_mimo_scattered_pilot_encoding=" << preamble.L1B_mimo_scattered_pilot_encoding << "\n";
        file << "L1B_lls_flag=" << preamble.sub_frames[0].plps[0].L1D_plp_lls_flag << "\n";
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
        std::cout << "Full configuration saved in: " << CONFIG_FILE << std::endl;
    } else {
        std::cerr << "ERROR: Failed to save configuration file. " << CONFIG_FILE << std::endl;
    }
}
