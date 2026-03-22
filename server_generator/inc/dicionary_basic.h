#ifndef DICIONARY_BASIC_H
#define DICIONARY_BASIC_H

#include <unordered_map>
#include <string>
#include <iostream>

struct syntax_l1_basic {
    int8_t L1B_version = 3;
    int8_t L1B_mimo_scattered_pilot_encoding = 1;
    int8_t L1B_lls_flag = 1;
    int8_t L1B_time_info_flag = 2;
    int8_t L1B_return_channel_flag = 1;
    int8_t L1B_papr_reduction = 2;
    int8_t L1B_frame_length_mode = 1;
    int16_t L1B_frame_length = 10;
    int16_t L1B_excess_samples_per_symbol = 13;
    int16_t L1B_time_offset = 16;
    int8_t L1B_additional_samples = 7;
    int8_t L1B_num_subframes = 8;
    int8_t L1B_preamble_num_symbols = 3;
    int8_t L1B_preamble_reduced_carriers = 3;
    int8_t L1B_L1_Detail_content_tag = 2;
    int16_t L1B_L1_Detail_size_bytes = 13;
    int8_t L1B_L1_Detail_fec_type = 3;
    int8_t L1B_L1_Detail_additional_parity_mode = 2;
    int32_t L1B_L1_Detail_total_cells = 19;
    int8_t L1B_first_sub_mimo = 1;
    int8_t L1B_first_sub_miso = 2;
    int8_t L1B_first_sub_fft_size = 2;
    int8_t L1B_first_sub_reduced_carriers = 3;
    int8_t L1B_first_sub_guard_interval = 4;
    int8_t L1B_first_sub_num_ofdm_symbols = 11;
    int8_t L1B_first_sub_scattered_pilot_pattern = 5;
    int8_t L1B_first_sub_scattered_pilot_boost = 3;
    int8_t L1B_first_sub_sbs_first = 1;
    int8_t L1B_first_sub_sbs_last = 1;
    int8_t L1B_first_sub_mimo_mixed = 1;
    int64_t L1B_reserved = 47;
    int32_t L1B_crc = 32;
};

const std::unordered_map<std::string, std::string> L1B_version_dicionary = {
    {"000", "Version 0"},
    {"001", "Version 1"},
    {"010", "Version 2"},
    {"011", "Version 3"},
    {"100", "Version 4"},
    {"101", "Version 5"},
    {"110", "Version 6"},
    {"111", "Version 7"}
};

const std::unordered_map<std::string, std::string> L1B_mimo_scattered_pilot_encoding_dicionary = {
    {"0", "Walsh-Hadamard pilots or no MIMO subframes"},
    {"1", "Null pilots"},
};

const std::unordered_map<std::string, std::string> L1B_lls_flag_dicionary = {
    {"0", "No LLS signaling in the current frame"},
    {"1", "There is LLS signaling carried in this frame"},
};

const std::unordered_map<std::string, std::string> L1B_time_info_flag_dicionary = {
    {"00", "Time information is not included in the current frame"},
    {"01", "Time information is included in the current frame and signaled to ms precision"},
    {"10", "Time information is included in the current frame and signaled to µs precision"},
    {"11", "Time information is included in the current frame and signaled to ns precision"},
};

const std::unordered_map<std::string, std::string> L1B_return_channel_flag_dicionary = {
	{"0", "DRC is not supported in the current frame of the current frequency band and current broadcast network"},
    {"1", "DRC is supported in the current frame of the current frequency band and current broadcast network"},
};

const std::unordered_map<std::string, std::string> L1B_papr_reduction_dicionary = {
    {"00", "No PAPR reduction used"},
    {"01", "Tone reservation only"},
    {"10", "ACE only"},
    {"11", "Both TR and ACE"},
};

const std::unordered_map<std::string, std::string> L1B_frame_length_mode_dicionary = {
	{"0", "The current frame is time-aligned with excess sample distribution"},
    {"1", "The current frame is symbol-aligned with no excess sample distribution"},
};

const std::unordered_map<std::string, std::string> L1B_L1_Detail_fec_type_dicionary = {
    {"000", "Mode 1"},
    {"001", "Mode 2"},
    {"010", "Mode 3"},
    {"011", "Mode 4"},
    {"100", "Mode 5"},
    {"101", "Mode 6"},
    {"110", "Mode 7"},
    {"111", "Reserved"}
};

const std::unordered_map<std::string, std::string> L1B_L1_Detail_additional_parity_mode_dicionary = {
    {"00", "K = 0"},
    {"01", "K = 1"},
    {"10", "K = 2"},
    {"11", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_fft_size_dicionary = {
    {"00", "8K"},
    {"01", "16K"},
    {"10", "32K"},
    {"11", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_mimo_dicionary = {
	{"0", "MIMO is not used"},
    {"1", "MIMO is used"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_miso_dicionary = {
    {"00", "No MISO"},
    {"01", "MISO with 64 coefficients"},
    {"10", "MISO with 256 coefficients"},
    {"11", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_guard_interval_dicionary = {
		{"0000", "Reserved"},
		{"0001", "GI1_192"},
		{"0010", "GI2_384"},
		{"0011", "GI3_512"},
		{"0100", "GI4_768"},
		{"0101", "GI5_1024"},
		{"0110", "GI6_1536"},
		{"0111", "GI7_2048"},
		{"1000", "GI8_2432"},
		{"1001", "GI9_3072"},
		{"1010", "GI10_3648"},
		{"1011", "GI11_4096"},
		{"1100", "GI12_4864"},
		{"1101", "Reserved"},
		{"1110", "Reserved"},
		{"1111", "Reserved"},
};

std::string L1D_scattered_pilot_boost_matrix_power[16][8] = {
		{"0.00","0.00","1.40","2.20","2.90","RFU","RFU","RFU"},
		{"0.00","1.40","2.90","3.80","4.40","RFU","RFU","RFU"},
		{"0.00","0.60","2.10","3.00","3.60","RFU","RFU","RFU"},
		{"0.00","2.10","3.60","4.40","5.10","RFU","RFU","RFU"},
		{"0.00","1.60","3.10","4.00","4.60","RFU","RFU","RFU"},
		{"0.00","3.00","4.50","5.40","6.00","RFU","RFU","RFU"},
		{"0.00","2.20","3.80","4.60","5.30","RFU","RFU","RFU"},
		{"0.00","3.60","5.10","6.00","6.60","RFU","RFU","RFU"},
		{"0.00","3.20","4.70","5.60","6.20","RFU","RFU","RFU"},
		{"0.00","4.50","6.00","6.90","7.50","RFU","RFU","RFU"},
		{"0.00","3.80","5.30","6.20","6.80","RFU","RFU","RFU"},
		{"0.00","5.20","6.70","7.60","8.20","RFU","RFU","RFU"},
		{"0.00","4.70","6.20","7.10","7.70","RFU","RFU","RFU"},
		{"0.00","6.10","7.60","8.50","9.10","RFU","RFU","RFU"},
		{"0.00","5.40","6.90","7.70","8.40","RFU","RFU","RFU"},
		{"0.00","6.70","8.20","9.10","9.70","RFU","RFU","RFU"},

};

std::string L1D_scattered_pilot_boost_matrix_amplitude[16][8] = {
		{"1.000","1.000","1.175","1.288","1.396","RFU","RFU","RFU"},
		{"1.000","1.175","1.396","1.549","1.660","RFU","RFU","RFU"},
		{"1.000","1.072","1.274","1.413","1.514","RFU","RFU","RFU"},
		{"1.000","1.274","1.514","1.660","1.799","RFU","RFU","RFU"},
		{"1.000","1.202","1.429","1.585","1.698","RFU","RFU","RFU"},
		{"1.000","1.413","1.679","1.862","1.995","RFU","RFU","RFU"},
		{"1.000","1.288","1.549","1.698","1.841","RFU","RFU","RFU"},
		{"1.000","1.514","1.799","1.995","2.138","RFU","RFU","RFU"},
		{"1.000","1.445","1.718","1.905","2.042","RFU","RFU","RFU"},
		{"1.000","1.679","1.995","2.213","2.371","RFU","RFU","RFU"},
		{"1.000","1.549","1.841","2.042","2.188","RFU","RFU","RFU"},
		{"1.000","1.820","2.163","2.399","2.570","RFU","RFU","RFU"},
		{"1.000","1.718","2.042","2.265","2.427","RFU","RFU","RFU"},
		{"1.000","2.018","2.399","2.661","2.851","RFU","RFU","RFU"},
		{"1.000","1.862","2.213","2.427","2.630","RFU","RFU","RFU"},
		{"1.000","2.163","2.570","2.851","3.055","RFU","RFU","RFU"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_sbs_first_dicionary = {
	{"0", "The first symbol of the first subframe of the current frame is not a subframe boundary symbol"},
    {"1", "The first symbol of the first subframe of the current frame is a subframe boundary symbol"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_sbs_last_dicionary = {
	{"0", "The last symbol of the first subframe of the current frame is not a subframe boundary symbol"},
    {"1", "The last symbol of the first subframe of the current frame is a subframe boundary symbol"},
};

const std::unordered_map<std::string, std::string> L1B_preamble_reduced_carriers_dicionary = {
	    {"000", "\n Number of Carriers: \n  8K FFT: 6913\n  16K FFT: 13825\n  32K FFT: 27649\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.832844\n  bsr_coefficient = 5: 6.804984\n  bsr_coefficient = 8: 7.777125"},
	    {"001", "\n Number of Carriers: \n  8K FFT: 6817\n  16K FFT: 13825\n  32K FFT: 27265\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.751844\n  bsr_coefficient = 5: 6.710484\n  bsr_coefficient = 8: 7.669125"},
	    {"010", "\n Number of Carriers: \n  8K FFT: 6721\n  16K FFT: 13441\n  32K FFT: 26881\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.670844\n  bsr_coefficient = 5: 6.615984\n  bsr_coefficient = 8: 7.561125"},
	    {"011", "\n Number of Carriers: \n  8K FFT: 6625\n  16K FFT: 13249\n  32K FFT: 26497\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.589844\n  bsr_coefficient = 5: 6.521484\n  bsr_coefficient = 8: 7.453125"},
	    {"100", "\n Number of Carriers: \n  8K FFT: 6529\n  16K FFT: 13057\n  32K FFT: 26113\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.508844\n  bsr_coefficient = 5: 6.426984\n  bsr_coefficient = 8: 7.345125"},
	    {"101", "Reserved"},
	    {"110", "Reserved"},
	    {"111", "Reserved"}

};

#endif
