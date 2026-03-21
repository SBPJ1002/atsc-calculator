#ifndef DICIONARY_DETAIL_H
#define DICIONARY_DETAIL_H

#include <unordered_map>
#include <string>
#include <iostream>

struct syntax_l1_detail {
    int8_t L1D_version = 4;
    int8_t L1D_num_rf = 3;
    int16_t L1D_bonded_bsid = 16;
    int8_t reserved = 3;
    int32_t L1D_time_sec = 32;
    int16_t L1D_time_msec = 10;
    int16_t L1D_time_usec = 10;
    int16_t L1D_time_nsec = 10;
    int8_t L1D_mimo = 1;
    int8_t L1D_miso = 2;
    int8_t L1D_fft_size = 2;
    int8_t L1D_reduced_carriers = 3;
    int8_t L1D_guard_interval = 4;
    int16_t L1D_num_ofdm_symbols = 11;
    int8_t L1D_scattered_pilot_pattern = 5;
    int8_t L1D_scattered_pilot_boost = 3;
    int8_t L1D_sbs_first = 1;
    int8_t L1D_sbs_last = 1;
    int8_t L1D_subframe_multiplex = 1;
    int8_t L1D_frequency_interleaver = 1;
    int16_t L1D_sbs_null_cells = 13;
    int8_t L1D_num_plp = 6;
    int8_t L1D_plp_id = 6;
    int8_t L1D_plp_lls_flag = 1;
    int8_t L1D_plp_layer = 2;
    int32_t L1D_plp_start = 24;
    int32_t L1D_plp_size = 24;
    int8_t L1D_plp_scrambler_type = 2;
    int8_t L1D_plp_fec_type = 4;
    int8_t L1D_plp_mod = 4;
    int8_t L1D_plp_cod = 4;
    int8_t L1D_plp_TI_mode = 2;
    int16_t L1D_plp_fec_block_start = 15;
    int32_t L1D_plp_CTI_fec_block_start = 22;
    int8_t L1D_plp_num_channel_bonded = 3;
    int8_t L1D_plp_channel_bonding_format = 2;
    int8_t L1D_plp_bonded_rf_id = 3;
    int8_t L1D_plp_mimo_stream_combining = 1;
    int8_t L1D_plp_mimo_IQ_interleaving = 1;
    int8_t L1D_plp_mimo_PH = 1;
    int8_t L1D_plp_type = 1;
    int16_t L1D_plp_num_subslices = 14;
    int32_t L1D_plp_subslice_interval = 24;
    int8_t L1D_plp_TI_extended_interleaving = 1;
	int8_t L1D_plp_CTI_depth = 3;
	int16_t L1D_plp_CTI_start_row = 11;
	int8_t L1D_plp_HTI_inter_subframe = 1;
	int8_t L1D_plp_HTI_num_ti_blocks = 4;
	int16_t L1D_plp_HTI_num_fec_blocks_max = 12;
	int16_t L1D_plp_HTI_num_fec_blocks = 12;
	int8_t L1D_plp_HTI_cell_interleaver = 1;
	int8_t L1D_plp_ldm_injection_level = 5;
	int16_t L1D_bsid = 16;
	int8_t L1D_mimo_mixed = 1;
	int8_t L1D_plp_mimo = 1;

	int32_t L1D_crc = 32;
};

const std::unordered_map<std::string, std::string> L1D_version_dicionary = {
    {"0000", "Version 0"},
    {"0001", "Version 1"},
    {"0010", "Version 2"},
    {"0011", "Version 3"},
    {"0100", "Version 4"},
    {"0101", "Version 5"},
    {"0110", "Version 6"},
    {"0111", "Version 7"},
	{"1000", "Version 8"},
	{"1001", "Version 8"},
	{"1010", "Version 10"},
	{"1011", "Version 11"},
	{"1100", "Version 12"},
	{"1101", "Version 13"},
	{"1110", "Version 14"},
	{"1111", "Version 15"}
};

const std::unordered_map<std::string, std::string> L1D_mimo_dicionary = {
	{"0", "The subframe includes one or more PLPs to which MIMO processing is not applied"},
    {"1", "MIMO processing is applied to all PLPs in the current subframe"},
};

const std::unordered_map<std::string, std::string> L1D_miso_dicionary = {
    {"00", "No MISO"},
	{"01", "MISO with 64 coefficients"},
	{"10", "MISO with 256 coefficients"},
	{"11", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_fft_size_dicionary = {
    {"00", "8K"},
    {"01", "16K"},
    {"10", "32K"},
    {"11", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_reduced_carriers_dicionary = {
	    {"000", "\n Number of Carriers: \n  8K FFT: 6913\n  16K FFT: 13825\n  32K FFT: 27649\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.832844\n  bsr_coefficient = 5: 6.804984\n  bsr_coefficient = 8: 7.777125"},
	    {"001", "\n Number of Carriers: \n  8K FFT: 6817\n  16K FFT: 13825\n  32K FFT: 27265\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.751844\n  bsr_coefficient = 5: 6.710484\n  bsr_coefficient = 8: 7.669125"},
	    {"010", "\n Number of Carriers: \n  8K FFT: 6721\n  16K FFT: 13441\n  32K FFT: 26881\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.670844\n  bsr_coefficient = 5: 6.615984\n  bsr_coefficient = 8: 7.561125"},
	    {"011", "\n Number of Carriers: \n  8K FFT: 6625\n  16K FFT: 13249\n  32K FFT: 26497\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.589844\n  bsr_coefficient = 5: 6.521484\n  bsr_coefficient = 8: 7.453125"},
	    {"100", "\n Number of Carriers: \n  8K FFT: 6529\n  16K FFT: 13057\n  32K FFT: 26113\n Occupied Bandwidth: \n  bsr_coefficient = 2: 5.508844\n  bsr_coefficient = 5: 6.426984\n  bsr_coefficient = 8: 7.345125"},
	    {"101", "Reserved"},
	    {"110", "Reserved"},
	    {"111", "Reserved"}

};

const std::unordered_map<std::string, std::string> L1D_guard_interval_dicionary = {
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

const std::unordered_map<std::string, std::string> L1D_scattered_pilot_pattern_dicionary_siso = {
		{"00000", "SP3_2"},
		{"00001", "SP3_4"},
		{"00010", "SP4_2"},
		{"00011", "SP4_4"},
		{"00100", "SP6_2"},
		{"00101", "SP6_4"},
		{"00110", "SP8_2"},
		{"00111", "SP8_4"},
		{"01000", "SP12_2"},
		{"01001", "SP12_4"},
		{"01010", "SP16_2"},
		{"01011", "SP16_4"},
		{"01100", "SP24_2"},
		{"01101", "SP24_4"},
		{"01110", "SP32_2"},
		{"01111", "SP32_4"},
		{"10000", "Reserved"},
		{"10001", "Reserved"},
		{"10010", "Reserved"},
		{"10011", "Reserved"},
		{"10100", "Reserved"},
		{"10101", "Reserved"},
		{"10110", "Reserved"},
		{"10111", "Reserved"},
		{"11000", "Reserved"},
		{"11001", "Reserved"},
		{"11010", "Reserved"},
		{"11011", "Reserved"},
		{"11100", "Reserved"},
		{"11101", "Reserved"},
		{"11110", "Reserved"},
		{"11111", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_scattered_pilot_pattern_dicionary_mimo = {
		{"00000", "MP3_2"},
		{"00001", "MP3_4"},
		{"00010", "MP4_2"},
		{"00011", "MP4_4"},
		{"00100", "MP6_2"},
		{"00101", "MP6_4"},
		{"00110", "MP8_2"},
		{"00111", "MP8_4"},
		{"01000", "MP12_2"},
		{"01001", "MP12_4"},
		{"01010", "MP16_2"},
		{"01011", "MP16_4"},
		{"01100", "MP24_2"},
		{"01101", "MP24_4"},
		{"01110", "MP32_2"},
		{"01111", "MP32_4"},
		{"10000", "Reserved"},
		{"10001", "Reserved"},
		{"10010", "Reserved"},
		{"10011", "Reserved"},
		{"10100", "Reserved"},
		{"10101", "Reserved"},
		{"10110", "Reserved"},
		{"10111", "Reserved"},
		{"11000", "Reserved"},
		{"11001", "Reserved"},
		{"11010", "Reserved"},
		{"11011", "Reserved"},
		{"11100", "Reserved"},
		{"11101", "Reserved"},
		{"11110", "Reserved"},
		{"11111", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_sbs_first_dicionary = {
	{"0", "The first symbol of the subframe is not a subframe boundary symbol"},
    {"1", "The first symbol of the subframe is a subframe boundary symbol"},
};

const std::unordered_map<std::string, std::string> L1D_sbs_last_dicionary = {
	{"0", "The last symbol of the subframe is not a subframe boundary symbol"},
    {"1", "The last symbol of the subframe is a subframe boundary symbol"},
};

const std::unordered_map<std::string, std::string> L1D_subframe_multiplex_dicionary = {
	{"0", "The current subframe is time-division multiplexed / concatenated in time with adjacent subframes"},
    {"1", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1D_frequency_interleaver_dicionary = {
	{"0", "The frequency interleaver is bypassed and not used"},
    {"1", "The frequency interleaver is enabled"},
};

const std::unordered_map<std::string, std::string> L1D_plp_lls_flag_dicionary = {
	{"0", "The current PLP does not carry LLS information"},
    {"1", "The current PLP does carry LLS information"},
};

const std::unordered_map<std::string, std::string> L1D_plp_layer_dicionary = {
    {"00", "Core Layer"},
	{"01", "Enhanced Layer"},
	{"10", "Enhanced Layer"},
	{"11", "Enhanced Layer"},
};

const std::unordered_map<std::string, std::string> L1D_plp_scrambler_type_dicionary = {
    {"00", "The value specifying the PRBS scramber type with polynomial [1 0 0 1 1 1 0 0 0 1 1 0 0 1 0 1] and initial state 0xF180, as defined in the applicable standard (e.g., DVB-S2X, DVB-T2)."},
	{"01", "Reserved for future use"},
	{"10", "Reserved for future use"},
	{"11", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1D_plp_fec_type_dicionary = {
		{"0000", "BCH + 16K LDPC"},
		{"0001", "BCH + 64K LDPC"},
		{"0010", "CRC + 16K LDPC"},
		{"0011", "CRC + 64K LDPC"},
		{"0100", "16K LDPC only"},
		{"0101", "64K LDPC only"},
		{"0110", "Reserved for future use"},
		{"0111", "Reserved for future use"},
		{"1000", "Reserved for future use"},
		{"1001", "Reserved for future use"},
		{"1010", "Reserved for future use"},
		{"1011", "Reserved for future use"},
		{"1100", "Reserved for future use"},
		{"1101", "Reserved for future use"},
		{"1110", "Reserved for future use"},
		{"1111", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1D_plp_mod_dicionary_siso = {
		{"0000", "QPSK"},
		{"0001", "16QAM-NUC"},
		{"0010", "64QAM-NUC"},
		{"0011", "256QAM-NUC"},
		{"0100", "1024QAM-NUC"},
		{"0101", "4096QAM-NUC"},
		{"0110", "Reserved"},
		{"0111", "Reserved"},
		{"1000", "Reserved"},
		{"1001", "Reserved"},
		{"1010", "Reserved"},
		{"1011", "Reserved"},
		{"1100", "Reserved"},
		{"1101", "Reserved"},
		{"1110", "Reserved"},
		{"1111", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_plp_mod_dicionary_mimo = {
		{"0000", "\n Bits per Cell Unit: 4 \nTx1 : QPSK \nTx2: QPSK"},
		{"0001", "\n Bits per Cell Unit: 8 \nTx1 : 16QAM-NUC \nTx2: 16QAM-NUC"},
		{"0010", "\n Bits per Cell Unit: 12 \nTx1 : 64QAM-NUC \nTx2: 64QAM-NUC"},
		{"0011", "\n Bits per Cell Unit: 16 \nTx1 : 256QAM-NUC \nTx2: 256QAM-NUC"},
		{"0100", "\n Bits per Cell Unit: 20 \nTx1 : 1024QAM-NUC \nTx2: 1024QAM-NUC"},
		{"0101", "\n Bits per Cell Unit: 24 \nTx1 : 4096QAM-NUC \nTx2: 4096QAM-NUC"},
		{"0110", "Reserved"},
		{"0111", "Reserved"},
		{"1000", "Reserved"},
		{"1001", "Reserved"},
		{"1010", "Reserved"},
		{"1011", "Reserved"},
		{"1100", "Reserved"},
		{"1101", "Reserved"},
		{"1110", "Reserved"},
		{"1111", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_plp_cod_dicionary = {
		{"0000", "2/15"},
		{"0001", "3/15"},
		{"0010", "4/15"},
		{"0011", "5/15"},
		{"0100", "6/15"},
		{"0101", "7/15"},
		{"0110", "8/15"},
		{"0111", "9/15"},
		{"1000", "10/15"},
		{"1001", "11/15"},
		{"1010", "12/15"},
		{"1011", "13/15"},
		{"1100", "Reserved"},
		{"1101", "Reserved"},
		{"1110", "Reserved"},
		{"1111", "Reserved"},
};

const std::unordered_map<std::string, std::string> L1D_plp_TI_mode_dicionary = {
    {"00", "No time interleaving mode (neither CTI nor HTI)"},
	{"01", "Convolutional time interleaving (CTI) mode"},
	{"10", "Hybrid time interleaving (HTI) mode"},
	{"11", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1D_plp_channel_bonding_format_dicionary = {
    {"00", "Plain channel bonding"},
	{"01", "SNR averaged channel bonding"},
	{"10", "Reserved for future use"},
	{"11", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1D_plp_mimo_stream_combining_dicionary = {
	{"0", "Stream combining option is not used"},
    {"1", "Stream combining is used"},
};

const std::unordered_map<std::string, std::string> L1D_plp_mimo_IQ_interleaving_dicionary = {
	{"0", "IQ polarization interleaving is not used"},
    {"1", "IQ polarization interleaving is used"},
};

const std::unordered_map<std::string, std::string> L1D_plp_mimo_PH_dicionary = {
	{"0", "Phase hopping is not used"},
    {"1", "Phase hopping is used"},
};

const std::unordered_map<std::string, std::string> L1D_plp_type_dicionary = {
	{"0", "PLP is non-dispersed"},
    {"1", "PLP is dispersed"},
};

const std::unordered_map<std::string, std::string> L1D_plp_TI_extended_interleaving_dicionary = {
	{"0", "Extended interleaving is not used"},
    {"1", "Extended interleaving is used"},
};

const std::unordered_map<std::string, std::string> L1D_plp_CTI_depth_dicionary = {
	{"000", "512"},
    {"001", "724"},
	{"010", "887 (non-extended interleaving) or 1254 (extended interleaving)"},
    {"011", "1024 (non-extended interleaving) or 1448 (extended interleaving)"},
	{"100", "Reserved for future use"},
    {"101", "Reserved for future use"},
	{"110", "Reserved for future use"},
    {"111", "Reserved for future use"},
};

const std::unordered_map<std::string, std::string> L1D_plp_ldm_injection_level_dicionary = {
	{"00000", "0.0"},
    {"00001", "0.5"},
	{"00010", "1.0"},
    {"00011", "1.5"},
	{"00100", "2.0"},
    {"00101", "2.5"},
	{"00110", "3.0"},
    {"00111", "3.5"},
	{"01000", "4.0"},
    {"01001", "4.5"},
	{"01010", "5.0"},
    {"01011", "6.0"},
	{"01100", "7.0"},
    {"01101", "8.0"},
	{"01110", "9.0"},
    {"01111", "10.0"},
	{"10000", "11.0"},
    {"10001", "12.0"},
	{"10010", "13.0"},
    {"10011", "14.0"},
	{"10100", "15.0"},
    {"10101", "16.0"},
	{"10110", "17.0"},
    {"10111", "18.0"},
	{"11000", "19.0"},
    {"11001", "20.0"},
	{"11010", "21.0"},
    {"11011", "22.0"},
	{"11100", "23.0"},
    {"11101", "24.0"},
	{"11110", "25.0"},
    {"11111", "Reserved"}
};

const std::unordered_map<std::string, std::string> L1D_plp_mimo_dicionary = {
	{"0", "MIMO is not used"},
    {"1", "MIMO is used"},
};

const std::unordered_map<std::string, std::string> L1B_first_sub_mimo_mixed_dicionary = {
	{"0", "PLPs in the subframe either use MIMO or do not use MIMO"},
    {"1", "PLPs using and not using MIMO are multiplexed within the subframe"},
};
#endif
