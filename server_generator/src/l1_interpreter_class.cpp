#include "l1_interpreter_class.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset>
#include <cstring>
#include <typeinfo>
#include <vector>
#include <stdexcept>
#include <iomanip>
#include <unordered_map>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "dicionary_basic.h"
#include "dicionary_detail.h"

L1_Interpreter::L1_Interpreter(ATSC_Config& cfg, const std::string& log_dir)
    : config(cfg), log_dir(log_dir), cursor(0), elements_processed(0) {}

std::string L1_Interpreter::process_segment(const std::string& array, size_t size, bool reset) {
    if (reset) {
        cursor = 0;
        elements_processed = 0;
    }
    elements_processed += size;
    std::string segment;
    size_t fim = cursor + size;
    if (fim > array.size()) {
        fim = array.size();
    }
    segment = array.substr(cursor, fim - cursor);
    cursor = fim;
    return segment;
}

void L1_Interpreter::interpret(const std::string& hex_value_detail, const std::string& hex_value_basic, const std::string& filename) {
	try {
	cursor = 0;
	elements_processed = 0;
	std::string binary_string_basic = BinaryUtils::hex_to_binary(hex_value_basic);
	const syntax_l1_basic l1b;

	    std::ofstream outFile(filename);
	    if (outFile.is_open()) {
	    	if (!outFile.is_open()) {
	    	        throw std::runtime_error("Could not create output file");
	    	    }

	    	outFile << "--- L1 Basic --- \n";
	    	std::cout << "--- L1 Basic --- \n";

	    	std::string L1B_version = process_segment(binary_string_basic, l1b.L1B_version);
	        outFile << "\nL1B_version → " << BinaryUtils::search_in_dicionary(L1B_version_dicionary,L1B_version) << "\n";
	        std::cout << "\nL1B_version → " << BinaryUtils::search_in_dicionary(L1B_version_dicionary,L1B_version) << "\n";

	        std::string L1B_mimo_scattered_pilot_encoding = process_segment(binary_string_basic, l1b.L1B_mimo_scattered_pilot_encoding);
	        outFile << "L1B_mimo_scattered_pilot_encoding → " << BinaryUtils::search_in_dicionary(L1B_mimo_scattered_pilot_encoding_dicionary,L1B_mimo_scattered_pilot_encoding) <<"\n";
	        std::cout << "L1B_mimo_scattered_pilot_encoding → " << BinaryUtils::search_in_dicionary(L1B_mimo_scattered_pilot_encoding_dicionary,L1B_mimo_scattered_pilot_encoding) <<"\n";

	        std::string L1B_lls_flag = process_segment(binary_string_basic, l1b.L1B_lls_flag);
	        outFile << "L1B_lls_flag → " << BinaryUtils::search_in_dicionary(L1B_lls_flag_dicionary,L1B_lls_flag)<< "\n";
	        std::cout << "L1B_lls_flag → " << BinaryUtils::search_in_dicionary(L1B_lls_flag_dicionary,L1B_lls_flag)<< "\n";

	        std::string L1B_time_info_flag = process_segment(binary_string_basic, l1b.L1B_time_info_flag);
	        outFile << "L1B_time_info_flag → " <<  BinaryUtils::search_in_dicionary(L1B_time_info_flag_dicionary,L1B_time_info_flag)<<"\n";
	        std::cout << "L1B_time_info_flag → " <<  BinaryUtils::search_in_dicionary(L1B_time_info_flag_dicionary,L1B_time_info_flag)<<"\n";

	        std::string L1B_return_channel_flag = process_segment(binary_string_basic, l1b.L1B_return_channel_flag);
	        outFile << "L1B_return_channel_flag → " << BinaryUtils::search_in_dicionary(L1B_return_channel_flag_dicionary,L1B_return_channel_flag) <<"\n";
	        std::cout << "L1B_return_channel_flag → " << BinaryUtils::search_in_dicionary(L1B_return_channel_flag_dicionary,L1B_return_channel_flag) <<"\n";

	        std::string L1B_papr_reduction = process_segment(binary_string_basic, l1b.L1B_papr_reduction);
	        outFile << "L1B_papr_reduction → " << BinaryUtils::search_in_dicionary(L1B_papr_reduction_dicionary,L1B_papr_reduction)<<"\n";
	        std::cout << "L1B_papr_reduction → " << BinaryUtils::search_in_dicionary(L1B_papr_reduction_dicionary,L1B_papr_reduction)<<"\n";

	        std::string L1B_frame_length_mode = process_segment(binary_string_basic, l1b.L1B_frame_length_mode);
	        outFile << "L1B_frame_length_mode → " <<  BinaryUtils::search_in_dicionary(L1B_frame_length_mode_dicionary,L1B_frame_length_mode)<<"\n";
	        std::cout << "L1B_frame_length_mode → " <<  BinaryUtils::search_in_dicionary(L1B_frame_length_mode_dicionary,L1B_frame_length_mode)<<"\n";

	        std::string field_name_one;
	        std::string field_value_one;
	        std::string field_name_two;
	        std::string field_value_two;
	        if (L1B_frame_length_mode == "0")
	        {
	        		field_name_one = "L1B_frame_length → ";
	        		std::string L1B_frame_length = process_segment(binary_string_basic, l1b.L1B_frame_length);
	        		field_value_one = L1B_frame_length;

	        		field_name_two = "L1B_excess_samples_per_symbol → ";
	        		std::string L1B_excess_samples_per_symbol = process_segment(binary_string_basic, l1b.L1B_excess_samples_per_symbol);
	        		field_value_two = L1B_excess_samples_per_symbol;
	        }
	        else
	        {
	        		field_name_one = "L1B_time_offset → ";
	           		std::string L1B_time_offset = process_segment(binary_string_basic, l1b.L1B_time_offset);
	        		field_value_one = L1B_time_offset;

	        		field_name_two = "L1B_additional_samples → ";
	        		std::string L1B_additional_samples = process_segment(binary_string_basic, l1b.L1B_additional_samples);
	        		field_value_two = L1B_additional_samples;
	     	}
	        outFile << field_name_one << BinaryUtils::bin_to_decimal(field_value_one)<<"\n";
	        std::cout << field_name_one << BinaryUtils::bin_to_decimal(field_value_one)<<"\n";
	        outFile << field_name_two << BinaryUtils::bin_to_decimal(field_value_two)<<"\n";
	        std::cout << field_name_two << BinaryUtils::bin_to_decimal(field_value_two)<<"\n";

	        std::string L1B_num_subframes = process_segment(binary_string_basic, l1b.L1B_num_subframes);
	        outFile << "L1B_num_subframes → " << BinaryUtils::bin_to_decimal(L1B_num_subframes)+1<<"\n";
	        std::cout << "L1B_num_subframes → " << BinaryUtils::bin_to_decimal(L1B_num_subframes)+1<<"\n";

	        std::string L1B_preamble_num_symbols = process_segment(binary_string_basic, l1b.L1B_preamble_num_symbols);
	        outFile << "L1B_preamble_num_symbols → " << BinaryUtils::bin_to_decimal(L1B_preamble_num_symbols) +1<<"\n";
	        std::cout << "L1B_preamble_num_symbols → " << BinaryUtils::bin_to_decimal(L1B_preamble_num_symbols) +1<<"\n";

	        std::string L1B_preamble_reduced_carriers = process_segment(binary_string_basic, l1b.L1B_preamble_reduced_carriers);
	        outFile << "L1B_preamble_reduced_carriers ↓ " << BinaryUtils::search_in_dicionary(L1B_preamble_reduced_carriers_dicionary,L1B_preamble_reduced_carriers) <<"\n";
	        std::cout << "L1B_preamble_reduced_carriers ↓ " << BinaryUtils::search_in_dicionary(L1B_preamble_reduced_carriers_dicionary,L1B_preamble_reduced_carriers) <<"\n";

	        std::string L1B_L1_Detail_content_tag = process_segment(binary_string_basic, l1b.L1B_L1_Detail_content_tag);
	        outFile << "L1B_L1_Detail_content_tag → " << BinaryUtils::bin_to_decimal(L1B_L1_Detail_content_tag) << "\n";
	        std::cout << "L1B_L1_Detail_content_tag → " << BinaryUtils::bin_to_decimal(L1B_L1_Detail_content_tag) << "\n";

	        std::string L1B_L1_Detail_size_bytes = process_segment(binary_string_basic, l1b.L1B_L1_Detail_size_bytes);
	        int L1B_L1_Detail_size_bytes_value = BinaryUtils::bin_to_decimal(L1B_L1_Detail_size_bytes);
	        int L1B_L1_Detail_size_bits = L1B_L1_Detail_size_bytes_value * 8;
	        outFile << "L1B_L1_Detail_size_bytes → " << L1B_L1_Detail_size_bytes_value << " bytes (" << L1B_L1_Detail_size_bits << " bits)\n";
	        std::cout << "L1B_L1_Detail_size_bytes → " << L1B_L1_Detail_size_bytes_value << " bytes (" << L1B_L1_Detail_size_bits << " bits)\n";

	        std::string L1B_L1_Detail_fec_type = process_segment(binary_string_basic, l1b.L1B_L1_Detail_fec_type);
	        outFile << "L1B_L1_Detail_fec_type → " << BinaryUtils::search_in_dicionary(L1B_L1_Detail_fec_type_dicionary,L1B_L1_Detail_fec_type) << "\n";
	        std::cout << "L1B_L1_Detail_fec_type → " << BinaryUtils::search_in_dicionary(L1B_L1_Detail_fec_type_dicionary,L1B_L1_Detail_fec_type) << "\n";

	        std::string L1B_L1_Detail_additional_parity_mode = process_segment(binary_string_basic, l1b.L1B_L1_Detail_additional_parity_mode);
	        outFile << "L1B_L1_Detail_additional_parity_mode → " << BinaryUtils::search_in_dicionary(L1B_L1_Detail_additional_parity_mode_dicionary,L1B_L1_Detail_additional_parity_mode) << "\n";
	        std::cout << "L1B_L1_Detail_additional_parity_mode → " << BinaryUtils::search_in_dicionary(L1B_L1_Detail_additional_parity_mode_dicionary,L1B_L1_Detail_additional_parity_mode) << "\n";

	        std::string L1B_L1_Detail_total_cells = process_segment(binary_string_basic, l1b.L1B_L1_Detail_total_cells);
	        outFile << "L1B_L1_Detail_total_cells → " << BinaryUtils::bin_to_decimal(L1B_L1_Detail_total_cells) << "\n";
	        std::cout << "L1B_L1_Detail_total_cells → " << BinaryUtils::bin_to_decimal(L1B_L1_Detail_total_cells) << "\n";

	        std::string L1B_first_sub_mimo = process_segment(binary_string_basic, l1b.L1B_first_sub_mimo);
	        outFile << "L1B_first_sub_mimo → " << BinaryUtils::search_in_dicionary(L1B_first_sub_mimo_dicionary,L1B_first_sub_mimo) << "\n";
	        std::cout << "L1B_first_sub_mimo → " << BinaryUtils::search_in_dicionary(L1B_first_sub_mimo_dicionary,L1B_first_sub_mimo) << "\n";

	        std::string L1B_first_sub_miso = process_segment(binary_string_basic, l1b.L1B_first_sub_miso);
	        outFile << "L1B_first_sub_miso → " << BinaryUtils::search_in_dicionary(L1B_first_sub_miso_dicionary, L1B_first_sub_miso) << "\n";
	        std::cout << "L1B_first_sub_miso → " << BinaryUtils::search_in_dicionary(L1B_first_sub_miso_dicionary, L1B_first_sub_miso) << "\n";

	        std::string L1B_first_sub_fft_size = process_segment(binary_string_basic, l1b.L1B_first_sub_fft_size);
	        outFile << "L1B_first_sub_fft_size → " << BinaryUtils::search_in_dicionary(L1B_first_sub_fft_size_dicionary,L1B_first_sub_fft_size)  << "\n";
	        std::cout << "L1B_first_sub_fft_size → " << BinaryUtils::search_in_dicionary(L1B_first_sub_fft_size_dicionary,L1B_first_sub_fft_size)  << "\n";

	        std::string L1B_first_sub_reduced_carriers = process_segment(binary_string_basic, l1b.L1B_first_sub_reduced_carriers);
	        outFile << "L1B_first_sub_reduced_carriers → " << BinaryUtils::bin_to_decimal(L1B_first_sub_reduced_carriers) << "\n";
	        std::cout << "L1B_first_sub_reduced_carriers → " << BinaryUtils::bin_to_decimal(L1B_first_sub_reduced_carriers) << "\n";

	        std::string L1B_first_sub_guard_interval = process_segment(binary_string_basic, l1b.L1B_first_sub_guard_interval);
	        outFile << "L1B_first_sub_guard_interval → " << BinaryUtils::search_in_dicionary(L1B_first_sub_guard_interval_dicionary,L1B_first_sub_guard_interval) << "\n";
	        std::cout << "L1B_first_sub_guard_interval → " << BinaryUtils::search_in_dicionary(L1B_first_sub_guard_interval_dicionary,L1B_first_sub_guard_interval) << "\n";

	        std::string L1B_first_sub_num_ofdm_symbols = process_segment(binary_string_basic, l1b.L1B_first_sub_num_ofdm_symbols);
	        outFile << "L1B_first_sub_num_ofdm_symbols → " << BinaryUtils::bin_to_decimal(L1B_first_sub_num_ofdm_symbols) + 1 << "\n";
	        std::cout << "L1B_first_sub_num_ofdm_symbols → " << BinaryUtils::bin_to_decimal(L1B_first_sub_num_ofdm_symbols) + 1 << "\n";

	        std::string L1B_first_sub_scattered_pilot_pattern = process_segment(binary_string_basic, l1b.L1B_first_sub_scattered_pilot_pattern);
	        std::string L1B_first_sub_scattered_pilot_pattern_type;
	        if (L1B_first_sub_mimo == "0" and L1B_first_sub_miso == "00"){
	        		L1B_first_sub_scattered_pilot_pattern_type = BinaryUtils::search_in_dicionary(L1D_scattered_pilot_pattern_dicionary_siso,L1B_first_sub_scattered_pilot_pattern);
	        		outFile << "L1B_first_sub_scattered_pilot_pattern → " << L1B_first_sub_scattered_pilot_pattern_type << "\n";
	        		std::cout << "L1B_first_sub_scattered_pilot_pattern → " << L1B_first_sub_scattered_pilot_pattern_type << "\n";
	        		}
	        	if (L1B_first_sub_mimo == "1"){
	        		L1B_first_sub_scattered_pilot_pattern_type = BinaryUtils::search_in_dicionary(L1D_scattered_pilot_pattern_dicionary_mimo,L1B_first_sub_scattered_pilot_pattern);
	        		 outFile << "L1B_first_sub_scattered_pilot_pattern → " << L1B_first_sub_scattered_pilot_pattern_type << "\n";
	        		 std::cout << "L1B_first_sub_scattered_pilot_pattern → " << L1B_first_sub_scattered_pilot_pattern_type << "\n";
	        	}

	        std::string L1B_first_sub_scattered_pilot_boost = process_segment(binary_string_basic, l1b.L1B_first_sub_scattered_pilot_boost);
	        int adress_line_basic = BinaryUtils::bin_to_decimal(L1B_first_sub_scattered_pilot_pattern);
	        int adress_collum_basic = BinaryUtils::bin_to_decimal(L1B_first_sub_scattered_pilot_boost);

	        if(adress_line_basic < 16){
	        	outFile << "L1B_first_sub_scattered_pilot_boost(power) → " << L1D_scattered_pilot_boost_matrix_power[adress_line_basic][adress_collum_basic] << " dB\n";
	        	std::cout << "L1B_first_sub_scattered_pilot_boost(power) → " << L1D_scattered_pilot_boost_matrix_power[adress_line_basic][adress_collum_basic] << " dB\n";
	        	outFile << "L1B_first_sub_scattered_pilot_boost(amplitude) → " << L1D_scattered_pilot_boost_matrix_amplitude[adress_line_basic][adress_collum_basic] << "\n";
	        	std::cout << "L1B_first_sub_scattered_pilot_boost(amplitude) → " << L1D_scattered_pilot_boost_matrix_amplitude[adress_line_basic][adress_collum_basic] << "\n";

	        }
	        else{
	        	outFile << "L1B_first_sub_scattered_pilot_boost → Reserved \n";
	        	std::cout << "L1B_first_sub_scattered_pilot_boost → Reserved \n";
	        }

	        std::string L1B_first_sub_sbs_first = process_segment(binary_string_basic, l1b.L1B_first_sub_sbs_first);
	        outFile << "L1B_first_sub_sbs_first → " << BinaryUtils::search_in_dicionary(L1B_first_sub_sbs_first_dicionary,L1B_first_sub_sbs_first) << "\n";
	        std::cout << "L1B_first_sub_sbs_first → " << BinaryUtils::search_in_dicionary(L1B_first_sub_sbs_first_dicionary,L1B_first_sub_sbs_first) << "\n";

	        std::string L1B_first_sub_sbs_last = process_segment(binary_string_basic, l1b.L1B_first_sub_sbs_last);
	        outFile << "L1B_first_sub_sbs_last → " << BinaryUtils::search_in_dicionary(L1B_first_sub_sbs_last_dicionary,L1B_first_sub_sbs_last) << "\n";
	        std::cout << "L1B_first_sub_sbs_last → " << BinaryUtils::search_in_dicionary(L1B_first_sub_sbs_last_dicionary,L1B_first_sub_sbs_last) << "\n";

	        std::string L1B_first_sub_mimo_mixed = process_segment(binary_string_basic, l1b.L1B_first_sub_mimo_mixed);
	        outFile << "L1B_first_sub_mimo_mixed → " << BinaryUtils::search_in_dicionary(L1B_first_sub_mimo_mixed_dicionary,L1B_first_sub_mimo_mixed)<< "\n";
	        std::cout << "L1B_first_sub_mimo_mixed → " << BinaryUtils::search_in_dicionary(L1B_first_sub_mimo_mixed_dicionary,L1B_first_sub_mimo_mixed)<< "\n";

	        std::string L1B_reserved = process_segment(binary_string_basic, l1b.L1B_reserved);
	        outFile << "L1B_reserved → " << BinaryUtils::bin_to_decimal(L1B_reserved)<< "\n";
	        std::cout << "L1B_reserved → " << BinaryUtils::bin_to_decimal(L1B_reserved)<< "\n";

	        std::string L1B_crc = process_segment(binary_string_basic, l1b.L1B_crc);
	        outFile << "L1B_crc → " << BinaryUtils::bin_to_decimal(L1B_crc) << "\n";
	        std::cout << "L1B_crc → " << BinaryUtils::bin_to_decimal(L1B_crc) << "\n";

	        outFile << "\n-- L1 Detail --- \n";

	        std::string binary_string_detail = BinaryUtils::hex_to_binary(hex_value_detail);
	        const syntax_l1_detail l1d;

	        std::string L1D_version = process_segment(binary_string_detail, l1d.L1D_version, true);
	                outFile << "L1D_version → " << BinaryUtils::search_in_dicionary(L1D_version_dicionary,L1D_version) << "\n";
	                std::cout << "L1D_version → " << BinaryUtils::search_in_dicionary(L1D_version_dicionary,L1D_version) << "\n";

	                std::string L1D_num_rf = process_segment(binary_string_detail, l1d.L1D_num_rf);
	                int L1D_num_rf_dec = BinaryUtils::bin_to_decimal(L1D_num_rf);
	                outFile << "L1D_num_rf → " << L1D_num_rf_dec << "\n";
	                std::cout << "L1D_num_rf → " << L1D_num_rf_dec << "\n";

	                for(int L1D_rf_id = 1; L1D_rf_id <= L1D_num_rf_dec; L1D_rf_id++){
	                	std::string L1D_bonded_bsid = process_segment(binary_string_detail, l1d.L1D_bonded_bsid);
	                	outFile << "L1D_bonded_bsid → " << BinaryUtils::bin_to_decimal(L1D_bonded_bsid) << "\n";
	                	std::cout << "L1D_bonded_bsid → " << BinaryUtils::bin_to_decimal(L1D_bonded_bsid) << "\n";

	                	std::string reserved = process_segment(binary_string_detail, l1d.reserved);
	                	outFile << "reserved → " << BinaryUtils::bin_to_decimal(reserved) << "\n";
	                	std::cout << "reserved → " << BinaryUtils::bin_to_decimal(reserved) << "\n";
	                }

	                int L1B_time_info_flag_dec = BinaryUtils::bin_to_decimal(L1B_time_info_flag);
	                if (L1B_time_info_flag_dec != 0){
	                	std::string L1D_time_sec = process_segment(binary_string_detail, l1d.L1D_time_sec);
	                	outFile << "L1D_time_sec → " <<  BinaryUtils::bin_to_decimal(L1D_time_sec) << "\n";
	                	std::cout << "L1D_time_sec → " <<  BinaryUtils::bin_to_decimal(L1D_time_sec) << "\n";

	                	std::string L1D_time_msec = process_segment(binary_string_detail, l1d.L1D_time_msec);
	                	outFile << "L1D_time_msec → " << BinaryUtils::bin_to_decimal(L1D_time_msec) << "\n";
	                	std::cout << "L1D_time_msec → " << BinaryUtils::bin_to_decimal(L1D_time_msec) << "\n";

	                	if(L1B_time_info_flag_dec != 1){
	                		std::string L1D_time_usec = process_segment(binary_string_detail, l1d.L1D_time_usec);
	                		outFile << "L1D_time_usec → " << BinaryUtils::bin_to_decimal(L1D_time_usec) << "\n";
	                		std::cout << "L1D_time_usec → " << BinaryUtils::bin_to_decimal(L1D_time_usec) << "\n";

	                		if(L1B_time_info_flag_dec != 2){
	                			std::string L1D_time_nsec = process_segment(binary_string_detail, l1d.L1D_time_nsec);
	                			outFile << "L1D_time_nsec → " << BinaryUtils::bin_to_decimal(L1D_time_nsec) << "\n";
	                			std::cout << "L1D_time_nsec → " << BinaryUtils::bin_to_decimal(L1D_time_nsec) << "\n";
	                		}
	                	}
	                }

	                int L1B_num_subframes_dec = BinaryUtils::bin_to_decimal(L1B_num_subframes);

	                std::string L1D_mimo;
	                std::string L1D_num_plp;
	                std::string L1D_sbs_first;;
	                std::string L1D_sbs_last;
	                std::string L1D_plp_mod;
	                int* L1D_num_plp_dec = nullptr;

	                for(int i = 0; i <= L1B_num_subframes_dec; i++){

	                	if(i > 0){

	                		L1D_mimo = process_segment(binary_string_detail, l1d.L1D_mimo);
	                		outFile << "L1D_mimo → " << BinaryUtils::search_in_dicionary(L1D_mimo_dicionary,L1D_mimo)  << "\n";
	                		std::cout << "L1D_mimo → " << BinaryUtils::search_in_dicionary(L1D_mimo_dicionary,L1D_mimo)  << "\n";

	                		std::string L1D_miso = process_segment(binary_string_detail, l1d.L1D_miso);
	                		outFile << "L1D_miso → " << BinaryUtils::search_in_dicionary(L1D_miso_dicionary,L1D_miso) << "\n";
	                		std::cout << "L1D_miso → " << BinaryUtils::search_in_dicionary(L1D_miso_dicionary,L1D_miso) << "\n";

	        	        	std::string L1D_fft_size = process_segment(binary_string_detail, l1d.L1D_fft_size);
	                		outFile << "L1D_fft_size → " << BinaryUtils::search_in_dicionary(L1D_fft_size_dicionary,L1D_fft_size) << "\n";
	                		std::cout << "L1D_fft_size → " << BinaryUtils::search_in_dicionary(L1D_fft_size_dicionary,L1D_fft_size) << "\n";

	                		std::string L1D_reduced_carriers = process_segment(binary_string_detail, l1d.L1D_reduced_carriers);
	                		outFile << "L1D_reduced_carriers ↓ " << BinaryUtils::search_in_dicionary(L1D_reduced_carriers_dicionary,L1D_reduced_carriers) << "\n";
	                		std::cout << "L1D_reduced_carriers ↓ " << BinaryUtils::search_in_dicionary(L1D_reduced_carriers_dicionary,L1D_reduced_carriers) << "\n";

	                		std::string L1D_guard_interval = process_segment(binary_string_detail, l1d.L1D_guard_interval);
	                		outFile << "L1D_guard_interval → " << BinaryUtils::search_in_dicionary(L1D_guard_interval_dicionary,L1D_guard_interval) << "\n";
	                		std::cout << "L1D_guard_interval → " << BinaryUtils::search_in_dicionary(L1D_guard_interval_dicionary,L1D_guard_interval) << "\n";

	                		std::string L1D_num_ofdm_symbols = process_segment(binary_string_detail, l1d.L1D_num_ofdm_symbols);
	                		outFile << "L1D_num_ofdm_symbols → " << BinaryUtils::bin_to_decimal(L1D_num_ofdm_symbols)+1 << "\n";
	                		std::cout << "L1D_num_ofdm_symbols → " << BinaryUtils::bin_to_decimal(L1D_num_ofdm_symbols)+1 << "\n";

	                		std::string L1D_scattered_pilot_pattern = process_segment(binary_string_detail, l1d.L1D_scattered_pilot_pattern);
	                		std::string L1D_scattered_pilot_pattern_type;
	                			        if (L1D_mimo == "0" and L1D_miso == "00"){
	                			        	L1D_scattered_pilot_pattern_type = BinaryUtils::search_in_dicionary(L1D_scattered_pilot_pattern_dicionary_siso,L1D_scattered_pilot_pattern);
              			        		 outFile << "L1D_scattered_pilot_pattern_siso → " << L1D_scattered_pilot_pattern_type << "\n";
              			        		 std::cout << "L1D_scattered_pilot_pattern_siso → " << L1D_scattered_pilot_pattern_type << "\n";
	                			        		}
	                			        if (L1D_mimo == "1"){
	                			        	L1D_scattered_pilot_pattern_type = BinaryUtils::search_in_dicionary(L1D_scattered_pilot_pattern_dicionary_mimo,L1D_scattered_pilot_pattern);
              			        		 outFile << "L1D_scattered_pilot_pattern_mimo → " << L1D_scattered_pilot_pattern_type << "\n";
              			        		 std::cout << "L1D_scattered_pilot_pattern_mimo → " << L1D_scattered_pilot_pattern_type << "\n";
	                			        	}
	                			        if (L1D_miso != "00"){
	                			        	outFile << "L1D_scattered_pilot_pattern → MISO is used\n";
	                			        	std::cout << "L1D_scattered_pilot_pattern → MISO is used\n";
	                			        }

	                	    if(L1D_miso == "00"){
	        					std::string L1D_scattered_pilot_boost = process_segment(binary_string_detail, l1d.L1D_scattered_pilot_boost);
	        					int adress_line_detail = BinaryUtils::bin_to_decimal(L1D_scattered_pilot_pattern);
	        					int adress_collum_detail = BinaryUtils::bin_to_decimal(L1D_scattered_pilot_boost);

	        					if(adress_line_detail < 16){
	        						outFile << "L1D_scattered_pilot_boost(power) → " << L1D_scattered_pilot_boost_matrix_power[adress_line_detail][adress_collum_detail] << " dB\n";
	        						std::cout << "L1D_scattered_pilot_boost(power) → " << L1D_scattered_pilot_boost_matrix_power[adress_line_detail][adress_collum_detail] << " dB\n";
	        						outFile << "L1D_scattered_pilot_boost(amplitude) → " << L1D_scattered_pilot_boost_matrix_amplitude[adress_line_detail][adress_collum_detail] << "\n";
	        						std::cout << "L1D_scattered_pilot_boost(amplitude) → " << L1D_scattered_pilot_boost_matrix_amplitude[adress_line_detail][adress_collum_detail] << "\n";
	        								}
	        					else{
	        						outFile << "L1B_first_sub_scattered_pilot_boost → Reserved \n";
	        						std::cout << "L1B_first_sub_scattered_pilot_boost → Reserved \n";
	        						}
	                	    }
	                	    L1D_sbs_first = process_segment(binary_string_detail, l1d.L1D_sbs_first);
	                		outFile << "L1D_sbs_first → " << BinaryUtils::search_in_dicionary(L1D_sbs_first_dicionary,L1D_sbs_first) << "\n";
	                		std::cout << "L1D_sbs_first → " << BinaryUtils::search_in_dicionary(L1D_sbs_first_dicionary,L1D_sbs_first) << "\n";

	                		L1D_sbs_last = process_segment(binary_string_detail, l1d.L1D_sbs_last);
	                		outFile << "L1D_sbs_last → " << BinaryUtils::search_in_dicionary(L1D_sbs_last_dicionary,L1D_sbs_last) << "\n";
	                		std::cout << "L1D_sbs_last → " << BinaryUtils::search_in_dicionary(L1D_sbs_last_dicionary,L1D_sbs_last) << "\n";
	                	}

	                	if (L1B_num_subframes_dec > 0){
	                		std::string L1D_subframe_multiplex = process_segment(binary_string_detail, l1d.L1D_subframe_multiplex);
	                		outFile << "L1D_subframe_multiplex → " << BinaryUtils::search_in_dicionary(L1D_subframe_multiplex_dicionary,L1D_subframe_multiplex) << "\n";
	                		std::cout << "L1D_subframe_multiplex → " << BinaryUtils::search_in_dicionary(L1D_subframe_multiplex_dicionary,L1D_subframe_multiplex) << "\n";
	                	}

	            		std::string L1D_frequency_interleaver = process_segment(binary_string_detail, l1d.L1D_frequency_interleaver);
	            		outFile << "L1D_frequency_interleaver → " << BinaryUtils::search_in_dicionary(L1D_frequency_interleaver_dicionary,L1D_frequency_interleaver) << "\n";
	            		std::cout << "L1D_frequency_interleaver → " << BinaryUtils::search_in_dicionary(L1D_frequency_interleaver_dicionary,L1D_frequency_interleaver) << "\n";

	            		int L1B_first_sub_sbs_first_dec = BinaryUtils::bin_to_decimal(L1B_first_sub_sbs_first);
	            		int L1B_first_sub_sbs_last_dec = BinaryUtils::bin_to_decimal(L1B_first_sub_sbs_last);

	        			int L1D_sbs_first_dec = 0;
	        			int L1D_sbs_last_dec = 0;

	        			if(i!=0)
	        			{
	        				L1D_sbs_first_dec = BinaryUtils::bin_to_decimal(L1D_sbs_first);
	        				L1D_sbs_last_dec = BinaryUtils::bin_to_decimal(L1D_sbs_last);
	        			}

	            		if (((i==0)&&(L1B_first_sub_sbs_first_dec || L1B_first_sub_sbs_last_dec)) || ((i>0) && (L1D_sbs_first_dec || L1D_sbs_last_dec))) {
	            			std::string L1D_sbs_null_cells = process_segment(binary_string_detail, l1d.L1D_sbs_null_cells);
	            			outFile << "L1D_sbs_null_cells → " <<BinaryUtils::bin_to_decimal(L1D_sbs_null_cells) << "\n";
	            			std::cout << "L1D_sbs_null_cells → " << L1D_sbs_null_cells << "→" << BinaryUtils::bin_to_decimal(L1D_sbs_null_cells) << "\n";
	            		}
	            		L1D_num_plp = process_segment(binary_string_detail, l1d.L1D_num_plp);
	            		L1D_num_plp_dec = new int(BinaryUtils::bin_to_decimal(L1D_num_plp));
	            		outFile << "L1D_num_plp → " << *L1D_num_plp_dec+1 << "\n";
	            		std::cout << "L1D_num_plp → " << *L1D_num_plp_dec+1 << "\n";
	            		for(int j=0; j<= *L1D_num_plp_dec; j++){
	            			std::string L1D_plp_id = process_segment(binary_string_detail, l1d.L1D_plp_id);
	            			outFile << " L1D_plp_id → " << BinaryUtils::bin_to_decimal(L1D_plp_id) << "\n";
	            			std::cout << " L1D_plp_id → " << BinaryUtils::bin_to_decimal(L1D_plp_id) << "\n";

	            			std::string L1D_plp_lls_flag = process_segment(binary_string_detail, l1d.L1D_plp_lls_flag);
	            			outFile << " 	L1D_plp_lls_flag → " << BinaryUtils::search_in_dicionary(L1D_plp_lls_flag_dicionary,L1D_plp_lls_flag) << "\n";
	            			std::cout << " 	L1D_plp_lls_flag → " << BinaryUtils::search_in_dicionary(L1D_plp_lls_flag_dicionary,L1D_plp_lls_flag) << "\n";

	            			std::string L1D_plp_layer = process_segment(binary_string_detail, l1d.L1D_plp_layer);
	            			outFile << " 	L1D_plp_layer → " << BinaryUtils::search_in_dicionary(L1D_plp_layer_dicionary,L1D_plp_layer) << "\n";
	            			std::cout << " 	L1D_plp_layer → " << BinaryUtils::search_in_dicionary(L1D_plp_layer_dicionary,L1D_plp_layer) << "\n";

	            			std::string L1D_plp_start = process_segment(binary_string_detail, l1d.L1D_plp_start);
	            			outFile << " 	L1D_plp_start → " << BinaryUtils::bin_to_decimal(L1D_plp_start) << "\n";
	            			std::cout << " 	L1D_plp_start → " << BinaryUtils::bin_to_decimal(L1D_plp_start) << "\n";

	            			std::string L1D_plp_size = process_segment(binary_string_detail, l1d.L1D_plp_size);
	            			outFile << " 	L1D_plp_size → " << BinaryUtils::bin_to_decimal(L1D_plp_size) << "\n";
	            			std::cout << " 	L1D_plp_size → " << BinaryUtils::bin_to_decimal(L1D_plp_size) << "\n";

	            			std::string L1D_plp_scrambler_type = process_segment(binary_string_detail, l1d.L1D_plp_scrambler_type);
	            			outFile << " 	L1D_plp_scrambler_type → " << BinaryUtils::search_in_dicionary(L1D_plp_scrambler_type_dicionary,L1D_plp_scrambler_type) << "\n";
	            			std::cout << " 	L1D_plp_scrambler_type → " << BinaryUtils::search_in_dicionary(L1D_plp_scrambler_type_dicionary,L1D_plp_scrambler_type) << "\n";

	            			std::string L1D_plp_fec_type = process_segment(binary_string_detail, l1d.L1D_plp_fec_type);
	            			int L1D_plp_fec_type_dec = BinaryUtils::bin_to_decimal(L1D_plp_fec_type);
	            			outFile << " 	L1D_plp_fec_type → " << BinaryUtils::search_in_dicionary(L1D_plp_fec_type_dicionary,L1D_plp_fec_type) << "\n";
	            			std::cout << " 	L1D_plp_fec_type → " << BinaryUtils::search_in_dicionary(L1D_plp_fec_type_dicionary,L1D_plp_fec_type) << "\n";

	            			if (L1D_plp_fec_type_dec >= 0 && L1D_plp_fec_type_dec <=5){
	            				L1D_plp_mod = process_segment(binary_string_detail, l1d.L1D_plp_mod);
	            				std::string L1D_plp_mod_type;
	            				if (L1B_first_sub_mimo == "0" and L1B_first_sub_miso == "00"){
	            					L1D_plp_mod_type = BinaryUtils::search_in_dicionary(L1D_plp_mod_dicionary_siso,L1D_plp_mod);
	            					outFile << " 	L1D_plp_mod → " << L1D_plp_mod_type << "\n";
	            					std::cout << " 	L1D_plp_mod → " << L1D_plp_mod_type << "\n";
	            				}
	            				if (L1B_first_sub_mimo == "1"){
	            					L1D_plp_mod_type = BinaryUtils::search_in_dicionary(L1D_plp_mod_dicionary_mimo,L1D_plp_mod);
	            					outFile << " 	L1D_plp_mod → " << L1D_plp_mod_type << "\n";
	            					std::cout << " 	L1D_plp_mod → " << L1D_plp_mod_type << "\n";
	            				}

	                			std::string L1D_plp_cod = process_segment(binary_string_detail, l1d.L1D_plp_cod);
	                			outFile << " 	L1D_plp_cod → " << BinaryUtils::search_in_dicionary(L1D_plp_cod_dicionary,L1D_plp_cod) << "\n";
	                			std::cout << " 	L1D_plp_cod → " << BinaryUtils::search_in_dicionary(L1D_plp_cod_dicionary,L1D_plp_cod) << "\n";
	            			}

	            			std::string L1D_plp_TI_mode = process_segment(binary_string_detail, l1d.L1D_plp_TI_mode);
	            			int L1D_plp_TI_mode_dec = BinaryUtils::bin_to_decimal(L1D_plp_TI_mode);
	            			outFile << " 	L1D_plp_TI_mode → " << BinaryUtils::search_in_dicionary(L1D_plp_TI_mode_dicionary,L1D_plp_TI_mode) << "\n";
	            			std::cout << " 	L1D_plp_TI_mode → " << BinaryUtils::search_in_dicionary(L1D_plp_TI_mode_dicionary,L1D_plp_TI_mode) << "\n";
	            			if (L1D_plp_TI_mode_dec == 0){
	            				std::string L1D_plp_fec_block_start = process_segment(binary_string_detail, l1d.L1D_plp_fec_block_start);
	            				outFile << " 	L1D_plp_fec_block_start → " << BinaryUtils::bin_to_decimal(L1D_plp_fec_block_start) << "\n";
	            				std::cout << " 	L1D_plp_fec_block_start → " << BinaryUtils::bin_to_decimal(L1D_plp_fec_block_start) << "\n";
	            			}
	            			else if(L1D_plp_TI_mode_dec == 1){
	            				std::string L1D_plp_CTI_fec_block_start = process_segment(binary_string_detail, l1d.L1D_plp_CTI_fec_block_start);
	            				outFile << " 	L1D_plp_CTI_fec_block_start → " << BinaryUtils::bin_to_decimal(L1D_plp_CTI_fec_block_start) << "\n";
	            				std::cout << " 	L1D_plp_CTI_fec_block_start → " << BinaryUtils::bin_to_decimal(L1D_plp_CTI_fec_block_start) << "\n";
	            			}

	            			if (L1D_num_rf_dec >0){
	            				std::string L1D_plp_num_channel_bonded = process_segment(L1D_plp_num_channel_bonded, l1d.L1D_plp_fec_block_start);
	            				int L1D_plp_num_channel_bonded_dec = BinaryUtils::bin_to_decimal(L1D_plp_num_channel_bonded);
	            				outFile << " 	L1D_plp_fec_block_start → " << L1D_plp_num_channel_bonded_dec << "\n";
	            				std::cout << " 	L1D_plp_fec_block_start → " << L1D_plp_num_channel_bonded_dec << "\n";

	            				if(L1D_plp_num_channel_bonded_dec > 0){
	            					std::string L1D_plp_channel_bonding_format = process_segment(L1D_plp_num_channel_bonded, l1d.L1D_plp_channel_bonding_format);
	            					outFile << " 	L1D_plp_channel_bonding_format → " << BinaryUtils::search_in_dicionary(L1D_plp_channel_bonding_format_dicionary,L1D_plp_channel_bonding_format) << "\n";
	            					std::cout << " 	L1D_plp_channel_bonding_format → " << BinaryUtils::search_in_dicionary(L1D_plp_channel_bonding_format_dicionary,L1D_plp_channel_bonding_format) << "\n";

	            					for (int k = 0; k<L1D_plp_num_channel_bonded_dec;k++){
	                    				std::string L1D_plp_bonded_rf_id = process_segment(binary_string_detail, l1d.L1D_plp_bonded_rf_id);
	                    				outFile << " 	L1D_plp_bonded_rf_id → " << BinaryUtils::bin_to_decimal(L1D_plp_bonded_rf_id) << "\n";
	                    				std::cout << " 	L1D_plp_bonded_rf_id → " << BinaryUtils::bin_to_decimal(L1D_plp_bonded_rf_id) << "\n";
	            					}
	            				}
	            			}

	            			if((i==0 && L1B_first_sub_mimo == "1")|| (i>0 && L1D_mimo == "1")){
	            				std::string L1D_plp_mimo_stream_combining = process_segment(binary_string_detail, l1d.L1D_plp_mimo_stream_combining);
	            				outFile << " 	L1D_plp_mimo_stream_combining → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_stream_combining_dicionary,L1D_plp_mimo_stream_combining) << "\n";
	            				std::cout << " 	L1D_plp_mimo_stream_combining → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_stream_combining_dicionary,L1D_plp_mimo_stream_combining) << "\n";

	            				std::string L1D_plp_mimo_IQ_interleaving = process_segment(binary_string_detail, l1d.L1D_plp_mimo_IQ_interleaving);
	            				outFile << " 	L1D_plp_mimo_IQ_interleaving → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_IQ_interleaving_dicionary,L1D_plp_mimo_IQ_interleaving) << "\n";
	            				std::cout << " 	L1D_plp_mimo_IQ_interleaving → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_IQ_interleaving_dicionary,L1D_plp_mimo_IQ_interleaving) << "\n";

	            				std::string L1D_plp_mimo_PH = process_segment(binary_string_detail, l1d.L1D_plp_mimo_PH);
	            				outFile << " 	L1D_plp_mimo_PH → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_PH_dicionary,L1D_plp_mimo_PH) << "\n";
	            				std::cout << " 	L1D_plp_mimo_PH → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_PH_dicionary,L1D_plp_mimo_PH) << "\n";
	            			}
	            			int L1D_plp_layer_dec = BinaryUtils::bin_to_decimal(L1D_plp_layer);
	            			if (L1D_plp_layer_dec == 0){
	            				std::string L1D_plp_type = process_segment(binary_string_detail, l1d.L1D_plp_mimo_PH);
	            				int L1D_plp_type_dec = BinaryUtils::bin_to_decimal(L1D_plp_type);
	            				outFile << " 	L1D_plp_type → " << BinaryUtils::search_in_dicionary(L1D_plp_type_dicionary,L1D_plp_type) << "\n";
	            				std::cout << " 	L1D_plp_type → " << BinaryUtils::search_in_dicionary(L1D_plp_type_dicionary,L1D_plp_type) << "\n";
	            				if (L1D_plp_type_dec == 1){
	            					std::string L1D_plp_num_subslices = process_segment(binary_string_detail, l1d.L1D_plp_num_subslices);
	            					outFile << " 	L1D_plp_num_subslices → " << BinaryUtils::bin_to_decimal(L1D_plp_num_subslices)+1 << "\n";
	            					std::cout << " 	L1D_plp_num_subslices → " << BinaryUtils::bin_to_decimal(L1D_plp_num_subslices)+1 << "\n";

	            					std::string L1D_plp_subslice_interval = process_segment(binary_string_detail, l1d.L1D_plp_subslice_interval);
	                				outFile << " 	L1D_plp_subslice_interval → " << BinaryUtils::bin_to_decimal(L1D_plp_subslice_interval) << "\n";
	                				std::cout << " 	L1D_plp_subslice_interval → " << BinaryUtils::bin_to_decimal(L1D_plp_subslice_interval) << "\n";
	            				}
	            				if (((L1D_plp_TI_mode == "01") ||(L1D_plp_TI_mode == "10"))&&(L1D_plp_mod == "0000")) {
	            					std::string L1D_plp_TI_extended_interleaving = process_segment(binary_string_detail, l1d.L1D_plp_TI_extended_interleaving);
	            					outFile << " 	L1D_plp_TI_extended_interleaving → " << BinaryUtils::search_in_dicionary(L1D_plp_TI_extended_interleaving_dicionary,L1D_plp_TI_extended_interleaving) << "\n";
	            					std::cout << " 	L1D_plp_TI_extended_interleaving → " << BinaryUtils::search_in_dicionary(L1D_plp_TI_extended_interleaving_dicionary,L1D_plp_TI_extended_interleaving) << "\n";
	            				}
	            				if(L1D_plp_TI_mode == "01"){
	            					std::string L1D_plp_CTI_depth = process_segment(binary_string_detail, l1d.L1D_plp_CTI_depth);
	            					outFile << " 	L1D_plp_CTI_depth → " << BinaryUtils::search_in_dicionary(L1D_plp_CTI_depth_dicionary,L1D_plp_CTI_depth) << "\n";
	            					std::cout << " 	L1D_plp_CTI_depth → " << BinaryUtils::search_in_dicionary(L1D_plp_CTI_depth_dicionary,L1D_plp_CTI_depth) << "\n";

	            					std::string L1D_plp_CTI_start_row = process_segment(binary_string_detail, l1d.L1D_plp_CTI_start_row);
	            					outFile << " 	L1D_ plp_CTI_start_row → " << BinaryUtils::bin_to_decimal(L1D_plp_CTI_start_row) << "\n";
	            					std::cout << " 	L1D_ plp_CTI_start_row → " << BinaryUtils::bin_to_decimal(L1D_plp_CTI_start_row) << "\n";

	            				}else if (L1D_plp_TI_mode == "10"){
	            					std::string L1D_plp_HTI_inter_subframe = process_segment(binary_string_detail, l1d.L1D_plp_HTI_inter_subframe);
	            					outFile << " 	L1D_plp_HTI_inter_subframe → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_inter_subframe) << "\n";
	            					std::cout << " 	L1D_plp_HTI_inter_subframe → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_inter_subframe) << "\n";

	            					std::string L1D_plp_HTI_num_ti_blocks = process_segment(binary_string_detail, l1d.L1D_plp_HTI_num_ti_blocks);
	            					int L1D_plp_HTI_num_ti_blocks_dec = BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_ti_blocks);
	            					outFile << " 	L1D_plp_HTI_num_ti_blocks → " << L1D_plp_HTI_num_ti_blocks_dec << "\n";
	            					std::cout << " 	L1D_plp_HTI_num_ti_blocks → " << L1D_plp_HTI_num_ti_blocks_dec << "\n";

	            					std::string L1D_plp_HTI_num_fec_blocks_max = process_segment(binary_string_detail, l1d.L1D_plp_HTI_num_fec_blocks_max);
	            					outFile << " 	L1D_plp_HTI_num_fec_blocks_max → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_fec_blocks_max) << "\n";
	            					std::cout << " 	L1D_plp_HTI_num_fec_blocks_max → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_fec_blocks_max) << "\n";

	            					if (L1D_plp_HTI_inter_subframe == "0"){
	                					std::string L1D_plp_HTI_num_fec_blocks = process_segment(binary_string_detail, l1d.L1D_plp_HTI_num_fec_blocks);
	                					outFile << " 	L1D_plp_HTI_num_fec_blocks → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_fec_blocks) << "\n";
	                					std::cout << " 	L1D_plp_HTI_num_fec_blocks → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_fec_blocks) << "\n";
	            					} else {
	            						for(int k =0; k<L1D_plp_HTI_num_ti_blocks_dec;k++){
	                    					std::string L1D_plp_HTI_num_fec_blocks = process_segment(binary_string_detail, l1d.L1D_plp_HTI_num_fec_blocks);
	                    					outFile << " 	L1D_plp_HTI_num_fec_blocks → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_fec_blocks) << "\n";
	                    					std::cout << " 	L1D_plp_HTI_num_fec_blocks → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_num_fec_blocks) << "\n";
	            						}
	            					}
	            					std::string L1D_plp_HTI_cell_interleaver = process_segment(binary_string_detail, l1d.L1D_plp_HTI_cell_interleaver);
	            					outFile << " 	L1D_plp_HTI_cell_interleaver → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_cell_interleaver) << "\n";
	            					std::cout << " 	L1D_plp_HTI_cell_interleaver → " << BinaryUtils::bin_to_decimal(L1D_plp_HTI_cell_interleaver) << "\n";
	            				}
	            			}else{
	        					std::string L1D_plp_ldm_injection_level = process_segment(binary_string_detail, l1d.L1D_plp_ldm_injection_level);
	        					outFile << " 	L1D_plp_ldm_injection_level → -" << BinaryUtils::search_in_dicionary(L1D_plp_ldm_injection_level_dicionary,L1D_plp_ldm_injection_level) << " dB\n";
	        					std::cout << " 	L1D_plp_ldm_injection_level → -" << BinaryUtils::search_in_dicionary(L1D_plp_ldm_injection_level_dicionary,L1D_plp_ldm_injection_level) << " dB\n";
	            			}
	            		}
	                }
	                if (L1D_version == "0001"){
	        			std::string L1D_bsid = process_segment(binary_string_detail, l1d.L1D_bsid);
	        			outFile << "L1D_bsid → " <<BinaryUtils::bin_to_decimal(L1D_bsid) << "\n";
	        			std::cout << "L1D_bsid → " <<BinaryUtils::bin_to_decimal(L1D_bsid) << "\n";
	                }
	                if(L1B_first_sub_mimo == "1"){
	                	for(int i = 0; i <= L1B_num_subframes_dec; i++){
	                					std::string L1D_mimo_mixed;
	                					if(i > 0){
	                						L1D_mimo_mixed = process_segment(binary_string_detail, l1d.L1D_mimo_mixed);
	                						outFile << "L1D_mimo_mixed → " << BinaryUtils::bin_to_decimal(L1D_mimo_mixed) << "\n";
	                						std::cout << "L1D_mimo_mixed → " << BinaryUtils::bin_to_decimal(L1D_mimo_mixed) << "\n";
	                					}if ((i == 0 && L1B_first_sub_mimo_mixed =="1") || (i >0 && L1D_mimo_mixed == "1")){
	                						for(int j=0; j<*L1D_num_plp_dec;j++){
	                							std::string L1D_plp_mimo = process_segment(binary_string_detail, l1d.L1D_plp_mimo);
	                							outFile << "L1D_plp_mimo → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_dicionary,L1D_plp_mimo) << "\n";
	                							std::cout << "L1D_plp_mimo → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_dicionary,L1D_plp_mimo) << "\n";

	                							if (L1D_plp_mimo == "1"){
	                								std::string L1D_plp_mimo_stream_combining = process_segment(binary_string_detail, l1d.L1D_plp_mimo_stream_combining);
	                								outFile << "L1D_plp_mimo_stream_combining → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_stream_combining_dicionary,L1D_plp_mimo_stream_combining) << "\n";
	                								std::cout << "L1D_plp_mimo_stream_combining → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_stream_combining_dicionary,L1D_plp_mimo_stream_combining) << "\n";

	                								std::string L1D_plp_mimo_IQ_interleaving = process_segment(binary_string_detail, l1d.L1D_plp_mimo_IQ_interleaving);
	                								outFile << "L1D_plp_mimo_IQ_interleaving → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_IQ_interleaving_dicionary,L1D_plp_mimo_IQ_interleaving) << "\n";
	                								std::cout << "L1D_plp_mimo_IQ_interleaving → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_IQ_interleaving_dicionary,L1D_plp_mimo_IQ_interleaving) << "\n";

	                								std::string L1D_plp_mimo_PH = process_segment(binary_string_detail, l1d.L1D_plp_mimo_PH);
	                								outFile << "L1D_plp_mimo_PH → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_PH_dicionary,L1D_plp_mimo_PH) << "\n";
	                								std::cout << "L1D_plp_mimo_PH → " << BinaryUtils::search_in_dicionary(L1D_plp_mimo_PH_dicionary,L1D_plp_mimo_PH) << "\n";
	                								}
	                							}
	                						}
	                					}
	                				}

	        		int total_L1D_bits = L1B_L1_Detail_size_bits;
	        		int crc_bits = 32;
	        		int processed_bits = elements_processed;
	        		int L1D_reserved_size = total_L1D_bits - processed_bits - crc_bits;

	        		if (L1D_reserved_size < 0){
	        			outFile << "ERRO: L1D_reserved negativo (" << L1D_reserved_size << " bits). L1D muito pequeno!\n";
	        			std::cout << "ERRO: L1D_reserved negativo (" << L1D_reserved_size << " bits). L1D muito pequeno!\n";
	        			L1D_reserved_size = 0;
	        		} else if (L1D_reserved_size == 0) {
	        			outFile << "L1D_reserved → 0 bits\n";
	        			std::cout << "L1D_reserved → 0 bits\n";
	        	} else {
	        		std::string L1D_reserved = process_segment(binary_string_detail, L1D_reserved_size);

	        		if (L1D_reserved_size > 63) {
	        			std::string hex_value = BinaryUtils::bin_to_hex_large(L1D_reserved);
	        			outFile << "L1D_reserved → " << hex_value << " (hexadecimal), "
	        			        << L1D_reserved_size << " bits\n";
	        			std::cout << "L1D_reserved → " << hex_value << " (hexadecimal), "
	        			          << L1D_reserved_size << " bits\n";
	        		} else {
	        			outFile << "L1D_reserved → " << BinaryUtils::bin_to_decimal(L1D_reserved) << " (decimal), "
	        			        << L1D_reserved_size << " bits\n";
	        			std::cout << "L1D_reserved → " << BinaryUtils::bin_to_decimal(L1D_reserved) << " (decimal), "
	        			          << L1D_reserved_size << " bits\n";
	        		}
	        	}

	        		std::string L1D_crc = process_segment(binary_string_detail, l1d.L1D_crc);
	        		outFile << "L1D_crc → " << BinaryUtils::bin_to_decimal(L1D_crc) << "\n";
	        		std::cout << "L1D_crc → " << BinaryUtils::bin_to_decimal(L1D_crc) << "\n";

	        		int total_processed = processed_bits + L1D_reserved_size + crc_bits;
	        		outFile << "\n[Verification] Total bits processed: " << total_processed
	        		        << ", Expected: " << total_L1D_bits;
	        		if (total_processed == total_L1D_bits) {
	        		    outFile << " OK\n";
	        		    std::cout << "\n[Verification] L1D size correct: " << total_L1D_bits << " bits\n";
	        		} else {
	        		    outFile << " ERRO! Diferenca: " << (total_L1D_bits - total_processed) << " bits\n";
	        		    std::cout << "\n[Verification] ERRO! Diferenca: " << (total_L1D_bits - total_processed) << " bits\n";
	        		}
		}

	        outFile.close();
	        std::cout << "Data saved at '" << filename << "' with success!\n";
	} catch (const std::overflow_error& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
	}}

void L1_Interpreter::clearLogDirectory(const std::string& logDir) {
    DIR* dir = opendir(logDir.c_str());
    if (dir == nullptr) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        std::string fullPath = logDir + "/" + name;
        remove(fullPath.c_str());
    }
    closedir(dir);
}

void L1_Interpreter::run() {
    std::cout << "\n=== Running L1 Interpreter ===" << std::endl;

    struct stat st;
    if (stat(log_dir.c_str(), &st) != 0) {
        mkdir(log_dir.c_str(), 0755);
    }

    clearLogDirectory(log_dir);

    std::ifstream basicStream(config.basic_file_binary, std::ios::binary);
    if (!basicStream.is_open()) {
        std::cerr << "Error: Could not open " << config.basic_file_binary << " for interpretation" << std::endl;
        return;
    }

    std::ifstream detailStream(config.detail_file_binary, std::ios::binary);
    if (!detailStream.is_open()) {
        basicStream.close();
        std::cerr << "Error: Could not open " << config.detail_file_binary << " for interpretation" << std::endl;
        return;
    }

    basicStream.seekg(0, std::ios::end);
    size_t basicFileSize = basicStream.tellg();
    basicStream.seekg(0, std::ios::beg);

    int numFrames = basicFileSize / L1_BASIC_SIZE_BYTES;
    std::cout << "  Frames to interpret: " << numFrames << std::endl;

    int processedCount = 0;
    int errorCount = 0;

    for (int frameNumber = 1; frameNumber <= numFrames; frameNumber++) {
        std::vector<unsigned char> basicData(L1_BASIC_SIZE_BYTES);
        basicStream.read(reinterpret_cast<char*>(basicData.data()), L1_BASIC_SIZE_BYTES);

        if (basicStream.gcount() != L1_BASIC_SIZE_BYTES) {
            std::cerr << "Warning: Incomplete L1 Basic data at frame " << frameNumber << std::endl;
            break;
        }

        int detailSize = BinaryUtils::extractL1DetailSize(basicData);

        std::vector<unsigned char> detailData(detailSize);
        detailStream.read(reinterpret_cast<char*>(detailData.data()), detailSize);

        if (detailStream.gcount() != detailSize) {
            std::cerr << "Warning: Incomplete L1 Detail data at frame " << frameNumber << std::endl;
            break;
        }

        std::string basicHex = BinaryUtils::bytesToHex(basicData);
        std::string detailHex = BinaryUtils::bytesToHex(detailData);
        std::string filename = log_dir + "/Frame_" + std::to_string(frameNumber) + ".log";

        try {
            interpret(detailHex, basicHex, filename);
            processedCount++;
        } catch (const std::exception& e) {
            std::cerr << "Error interpreting frame " << frameNumber << ": " << e.what() << std::endl;
            errorCount++;
        }
    }

    basicStream.close();
    detailStream.close();

    std::cout << "  Interpreter done: " << processedCount << " frames processed";
    if (errorCount > 0) {
        std::cout << ", " << errorCount << " errors";
    }
    std::cout << std::endl;
    std::cout << "  Logs saved to: " << log_dir << "/" << std::endl;
    std::cout << "==============================\n" << std::endl;
}
