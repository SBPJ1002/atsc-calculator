#include "functions.h"

std::string process_user_command(std::string command) {
    std::string reply = "";

    std::cout << "Processing: " << command << std::endl;

    command.erase(0, command.find_first_not_of(" \t\r\n"));
    command.erase(command.find_last_not_of(" \t\r\n") + 1);

    if (command.substr(0, 16) == "SET_FRAME_COUNT=") {
            global_frame_count = std::stoi(command.substr(16));
            reply = "SET_FRAME_COUNT Done\n";
            std::cout << "Frame Count = " << global_frame_count << std::endl;
        }

    if (command.substr(0, 10) == "GET_CONFIG") {
        std::cout << "Sending current configuration" << std::endl;
        reply += std::to_string(bootstrap_info.major_version) + "\n";
        reply += std::to_string(bootstrap_info.minor_version) + "\n";
        reply += std::to_string(bootstrap_info.bootstrap_symbol) + "\n";
        reply += std::to_string(bootstrap_info.ea_wakeup) + "\n";
        reply += std::to_string(bootstrap_info.system_bandwidth) + "\n";
        reply += std::to_string(bootstrap_info.bsr_coefficient) + "\n";
        reply += std::to_string(bootstrap_info.min_time_to_next) + "\n";
        reply += std::to_string(bootstrap_info.preamble_structure) + "\n";
        reply += std::to_string(global_frame_count) + "\n";


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
    else if (command.substr(0, 18) == "SET_MAJOR_VERSION=") {
        bootstrap_info.major_version = std::stoi(command.substr(18));
        reply = "SET_MAJOR_VERSION Done\n";
        std::cout << "Major Version = " << bootstrap_info.major_version << std::endl;
    }
    else if (command.substr(0, 18) == "SET_MINOR_VERSION=") {
        bootstrap_info.minor_version = std::stoi(command.substr(18));
        reply = "SET_MINOR_VERSION Done\n";
        std::cout << "Minor Version = " << bootstrap_info.minor_version << std::endl;
    }
    else if (command.substr(0, 21) == "SET_BOOTSTRAP_SYMBOL=") {
        bootstrap_info.bootstrap_symbol = std::stoi(command.substr(21));
        reply = "SET_BOOTSTRAP_SYMBOL Done\n";
        std::cout << "Bootstrap Symbol = " << bootstrap_info.bootstrap_symbol << std::endl;
    }
    else if (command.substr(0, 14) == "SET_EA_WAKEUP=") {
        bootstrap_info.ea_wakeup = std::stoi(command.substr(14));
        reply = "SET_EA_WAKEUP Done\n";
        std::cout << "EA Wakeup = " << bootstrap_info.ea_wakeup << std::endl;
    }
    else if (command.substr(0, 21) == "SET_SYSTEM_BANDWIDTH=") {
        bootstrap_info.system_bandwidth = std::stoi(command.substr(21));
        reply = "SET_SYSTEM_BANDWIDTH Done\n";
        std::cout << "System Bandwidth = " << bootstrap_info.system_bandwidth << std::endl;
    }
    else if (command.substr(0, 20) == "SET_BSR_COEFFICIENT=") {
        bootstrap_info.bsr_coefficient = std::stoi(command.substr(20));
        reply = "SET_BSR_COEFFICIENT Done\n";
        std::cout << "BSR Coefficient = " << bootstrap_info.bsr_coefficient << std::endl;
    }
    else if (command.substr(0, 21) == "SET_MIN_TIME_TO_NEXT=") {
        bootstrap_info.min_time_to_next = std::stoi(command.substr(21));
        reply = "SET_MIN_TIME_TO_NEXT Done\n";
        std::cout << "Min Time To Next = " << bootstrap_info.min_time_to_next << std::endl;
    }
    else if (command.substr(0, 23) == "SET_PREAMBLE_STRUCTURE=") {
        bootstrap_info.preamble_structure = std::stoi(command.substr(23));
        reply = "SET_PREAMBLE_STRUCTURE Done\n";
        std::cout << "Preamble Structure = " << bootstrap_info.preamble_structure << std::endl;
    }
    else if (command.substr(0, 16) == "SET_L1B_VERSION=") {
        preamble.L1B_version = std::stoi(command.substr(16));
        reply = "SET_L1B_VERSION Done\n";
        std::cout << "L1B Version = " << preamble.L1B_version << std::endl;
    }
    else if (command.substr(0, 39) == "SET_L1B_MIMO_SCATTERRED_PILOT_ENCODING=") {
        preamble.L1B_mimo_scattered_pilot_encoding = std::stoi(command.substr(39));
        reply = "SET_L1B_MIMO_SCATTERRED_PILOT_ENCODING Done\n";
        std::cout << "L1B MIMO Encoding = " << preamble.L1B_mimo_scattered_pilot_encoding << std::endl;
    }
    else if (command.substr(0, 20) == "SET_DETAIL_FEC_TYPE=") {
        preamble.L1B_L1_Detail_fec_type = std::stoi(command.substr(20));
        reply = "SET_DETAIL_FEC_TYPE Done\n";
        std::cout << "Detail FEC Type = " << preamble.L1B_L1_Detail_fec_type << std::endl;
    }
    else if (command.substr(0, 19) == "SET_TIME_INFO_FLAG=") {
        preamble.L1B_time_info_flag = std::stoi(command.substr(19));
        reply = "SET_TIME_INFO_FLAG Done\n";
        std::cout << "Time Info Flag = " << preamble.L1B_time_info_flag << std::endl;
    }
    else if (command.substr(0, 22) == "SET_FRAME_LENGHT_MODE=") {
        preamble.L1B_frame_length_mode = std::stoi(command.substr(22));
        reply = "SET_FRAME_LENGHT_MODE Done\n";
        std::cout << "Frame Length Mode = " << preamble.L1B_frame_length_mode << std::endl;
    }
    else if (command.substr(0, 17) == "SET_FRAME_LENGHT=") {
        preamble.L1B_frame_length = std::stoi(command.substr(17));
        reply = "SET_FRAME_LENGHT Done\n";
        std::cout << "Frame Length = " << preamble.L1B_frame_length << std::endl;
    }
    else if (command.substr(0, 24) == "SET_NUMBER_OF_SUBFRAMES=") {
        int new_count = std::stoi(command.substr(24));
        preamble.L1B_num_subframes = new_count;
        preamble.sub_frames.resize(new_count);
        reply = "SET_NUMBER_OF_SUBFRAMES Done\n";
        std::cout << "Number of Subframes = " << preamble.L1B_num_subframes << std::endl;
    }
    else if (command.substr(0, 25) == "SET_PREAMBLE_NUM_SYMBOLS=") {
        preamble.L1B_preamble_num_symbols = std::stoi(command.substr(25));
        reply = "SET_PREAMBLE_NUM_SYMBOLS Done\n";
        std::cout << "Preamble Num Symbols = " << preamble.L1B_preamble_num_symbols << std::endl;
    }
    else if (command.substr(0, 16) == "SET_L1D_VERSION=") {
        preamble.L1D_version = std::stoi(command.substr(16));
        reply = "SET_L1D_VERSION Done\n";
        std::cout << "L1D Version = " << preamble.L1D_version << std::endl;
    }
    else if (command.substr(0, 13) == "SET_L1D_BSID=") {
        int bsid_value = std::stoi(command.substr(13));
        preamble.L1D_bsid = bsid_value;
        reply = "SET_L1D_BSID Done\n";
        std::cout << "L1D BSID = " << preamble.L1D_bsid << std::endl;
    }
    else if (command.substr(0, 13) == "SET_SUBFRAME_") {
        size_t idx_pos = command.find('_', 13);
        if (idx_pos != std::string::npos) {
            int subframe_idx = std::stoi(command.substr(13, idx_pos - 13));
            std::string param_part = command.substr(idx_pos + 1);

            std::cout << "Subframe " << subframe_idx << " - " << param_part << std::endl;

            if (subframe_idx >= preamble.sub_frames.size()) {
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

                    if (plp_idx >= preamble.sub_frames[subframe_idx].plps.size()) {
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
        reply = "ERRO: Unknown command: " + command + "\n";
        std::cout << "Unknown command: " << command << std::endl;
    }

    return reply;
}
