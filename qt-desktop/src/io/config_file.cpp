#include "io/config_file.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QRegularExpression>

static int extractIndex(const QString& s) {
    int start = s.indexOf('[');
    int end = s.indexOf(']');
    if (start >= 0 && end > start)
        return s.mid(start + 1, end - start - 1).toInt();
    return -1;
}

namespace ConfigFile {

bool load(const QString& path, AtscConfig& config) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    config = AtscConfig();
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) continue;

        int eq = line.indexOf('=');
        if (eq < 0) continue;
        QString key = line.left(eq).trimmed();
        QString val = line.mid(eq + 1).trimmed();
        int v = val.toInt();

        if (key == "major_version") config.bootstrap.major_version = v;
        else if (key == "minor_version") config.bootstrap.minor_version = v;
        else if (key == "bootstrap_symbol") config.bootstrap.bootstrap_symbol = v;
        else if (key == "ea_wakeup") config.bootstrap.ea_wakeup = v;
        else if (key == "system_bandwidth") config.bootstrap.system_bandwidth = v;
        else if (key == "bsr_coefficient") config.bootstrap.bsr_coefficient = v;
        else if (key == "min_time_to_next") config.bootstrap.min_time_to_next = v;
        else if (key == "preamble_structure") config.bootstrap.preamble_structure = v;
        else if (key == "number_of_frames") config.bootstrap.number_of_frames = v;

        else if (key == "L1B_version") config.preamble.l1b_version = v;
        else if (key == "L1B_mimo_scattered_pilot_encoding") config.preamble.l1b_mimo_scattered_pilot_encoding = v;
        else if (key == "L1B_lls_flag") config.preamble.l1b_lls_flag = v;
        else if (key == "L1B_time_info_flag") config.preamble.l1b_time_info_flag = v;
        else if (key == "L1B_return_channel_flag") config.preamble.l1b_return_channel_flag = v;
        else if (key == "L1B_papr_reduction") config.preamble.l1b_papr_reduction = v;
        else if (key == "L1B_frame_length_mode") config.preamble.l1b_frame_length_mode = v;
        else if (key == "L1B_frame_length") config.preamble.l1b_frame_length = v;
        else if (key == "L1B_excess_samples_per_symbol") config.preamble.l1b_excess_samples_per_symbol = v;
        else if (key == "L1B_time_offset") config.preamble.l1b_time_offset = v;
        else if (key == "L1B_additional_samples") config.preamble.l1b_additional_samples = v;
        else if (key == "L1B_num_subframes") {
            config.preamble.l1b_num_subframes = v;
            config.subframes.resize(v);
        }
        else if (key == "L1B_preamble_num_symbols") config.preamble.l1b_preamble_num_symbols = v;
        else if (key == "L1B_preamble_reduced_carriers") config.preamble.l1b_preamble_reduced_carriers = v;
        else if (key == "L1B_L1_Detail_content_tag") config.preamble.l1b_detail_content_tag = v;
        else if (key == "L1B_L1_Detail_size_bytes") config.preamble.l1b_detail_size_bytes = v;
        else if (key == "L1B_L1_Detail_fec_type") config.preamble.l1b_detail_fec_type = v;
        else if (key == "L1B_L1_Detail_additional_parity_mode") config.preamble.l1b_detail_additional_parity_mode = v;
        else if (key == "L1B_L1_Detail_total_cells") config.preamble.l1b_detail_total_cells = v;
        else if (key == "L1D_version") config.preamble.l1d_version = v;
        else if (key == "L1D_bsid") config.preamble.l1d_bsid = v;
        else if (key == "L1D_num_rf") config.preamble.l1d_num_rf = v;
        else if (key == "L1D_rf_id") config.preamble.l1d_rf_id = v;
        else if (key == "L1D_bonded_bsid") config.preamble.l1d_bonded_bsid = v;
        else if (key == "L1D_time_sec") config.preamble.l1d_time_sec = v;
        else if (key == "L1D_time_msec") config.preamble.l1d_time_msec = v;
        else if (key == "L1D_time_usec") config.preamble.l1d_time_usec = v;
        else if (key == "L1D_time_nsec") config.preamble.l1d_time_nsec = v;

        else if (key.startsWith("subframe.")) {
            QString rest = key.mid(9);
            int dot1 = rest.indexOf('.');
            if (dot1 < 0) continue;
            int sfIdx = extractIndex(rest.left(dot1));
            if (sfIdx < 0) continue;
            QString field = rest.mid(dot1 + 1);

            if (sfIdx >= (int)config.subframes.size())
                config.subframes.resize(sfIdx + 1);
            auto& sf = config.subframes[sfIdx];

            if (field.startsWith("plp.")) {
                QString plpPart = field.mid(4);
                int dot2 = plpPart.indexOf('.');
                if (dot2 < 0) continue;
                int plpIdx = extractIndex(plpPart.left(dot2));
                if (plpIdx < 0) continue;
                QString prop = plpPart.mid(dot2 + 1);

                if (plpIdx >= (int)sf.plps.size())
                    sf.plps.resize(plpIdx + 1);
                auto& plp = sf.plps[plpIdx];

                if (prop == "L1D_plp_id") plp.id = v;
                else if (prop == "L1D_plp_lls_flag") plp.lls_flag = v;
                else if (prop == "L1D_plp_layer") plp.layer = v;
                else if (prop == "L1D_plp_start") plp.start = v;
                else if (prop == "L1D_plp_size") plp.size = v;
                else if (prop == "L1D_plp_scrambler_type") plp.scrambler_type = v;
                else if (prop == "L1D_plp_fec_type") plp.fec_type = v;
                else if (prop == "L1D_plp_mod") plp.mod = v;
                else if (prop == "L1D_plp_cod") plp.cod = v;
                else if (prop == "L1D_plp_TI_mode") plp.ti_mode = v;
                else if (prop == "L1D_plp_fec_block_start") plp.fec_block_start = v;
                else if (prop == "L1D_plp_CTI_fec_block_start") plp.cti_fec_block_start = v;
                else if (prop == "L1D_plp_num_channel_bonded") plp.num_channel_bonded = v;
                else if (prop == "L1D_plp_channel_bonding_format") plp.channel_bonding_format = v;
                else if (prop == "L1D_plp_bonded_rf_id") plp.bonded_rf_id = v;
                else if (prop == "L1D_plp_TI_extended_interleaving") plp.ti_extended_interleaving = v;
                else if (prop == "L1D_plp_CTI_depth") plp.cti_depth = v;
                else if (prop == "L1D_plp_CTI_start_row") plp.cti_start_row = v;
                else if (prop == "L1D_plp_mimo") plp.mimo = v;
                else if (prop == "L1D_plp_mimo_stream_combining") plp.mimo_stream_combining = v;
                else if (prop == "L1D_plp_mimo_IQ_interleaving") plp.mimo_iq_interleaving = v;
                else if (prop == "L1D_plp_mimo_PH") plp.mimo_ph = v;
                else if (prop == "L1D_plp_type") plp.type = v;
                else if (prop == "L1D_plp_num_subslices") plp.num_subslices = v;
                else if (prop == "L1D_plp_subslice_interval") plp.subslice_interval = v;
                else if (prop == "L1D_plp_HTI_inter_subframe") plp.hti_inter_subframe = v;
                else if (prop == "L1D_plp_HTI_num_ti_blocks") plp.hti_num_ti_blocks = v;
                else if (prop == "L1D_plp_HTI_num_fec_blocks_max") plp.hti_num_fec_blocks_max = v;
                else if (prop == "L1D_plp_HTI_num_fec_blocks") plp.hti_num_fec_blocks = v;
                else if (prop == "L1D_plp_HTI_cell_interleaver") plp.hti_cell_interleaver = v;
                else if (prop == "L1D_plp_ldm_injection_level") plp.ldm_injection_level = v;
            } else {

                if (field == "L1B_first_sub_mimo" || field == "L1D_mimo") sf.mimo = v;
                else if (field == "L1B_first_sub_mimo_mixed" || field == "L1D_mimo_mixed") sf.mimo_mixed = v;
                else if (field == "L1B_first_sub_miso" || field == "L1D_miso") sf.miso = v;
                else if (field == "L1B_first_sub_fft_size" || field == "L1D_fft_size") sf.fft_size = v;
                else if (field == "L1B_first_sub_reduced_carriers" || field == "L1D_reduced_carriers") sf.reduced_carriers = v;
                else if (field == "L1B_first_sub_guard_interval" || field == "L1D_guard_interval") sf.guard_interval = v;
                else if (field == "L1B_first_sub_num_ofdm_symbols" || field == "L1D_num_ofdm_symbols") sf.num_ofdm_symbols = v;
                else if (field == "L1B_first_sub_scattered_pilot_pattern" || field == "L1D_scattered_pilot_pattern") sf.scattered_pilot_pattern = v;
                else if (field == "L1B_first_sub_scattered_pilot_boost" || field == "L1D_scattered_pilot_boost") sf.scattered_pilot_boost = v;
                else if (field == "L1B_first_sub_sbs_first" || field == "L1D_sbs_first") sf.sbs_first = v;
                else if (field == "L1B_first_sub_sbs_last" || field == "L1D_sbs_last") sf.sbs_last = v;
                else if (field == "L1D_subframe_multiplex") sf.subframe_multiplex = v;
                else if (field == "L1D_frequency_interleaver") sf.frequency_interleaver = v;
                else if (field == "L1D_sbs_null_cells") sf.sbs_null_cells = v;
                else if (field == "L1D_num_plp") {
                    if (v > 0 && v > (int)sf.plps.size())
                        sf.plps.resize(v);
                }
            }
        }
    }

    file.close();
    config.ensureSubframes();
    return true;
}

bool save(const QString& path, const AtscConfig& config) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&file);
    const auto& b = config.bootstrap;
    const auto& p = config.preamble;

    out << "# =============================================\n";
    out << "# ATSC 3.0 Modulator Configuration File\n";
    out << "# Auto-generated - Do not edit manually\n";
    out << "# =============================================\n\n";
    out << "# Last updated: " << QDateTime::currentDateTime().toString() << "\n\n";

    out << "# =============================================\n";
    out << "# BOOTSTRAP CONFIGURATION\n";
    out << "# =============================================\n";
    out << "major_version=" << b.major_version << "\n";
    out << "minor_version=" << b.minor_version << "\n";
    out << "bootstrap_symbol=" << b.bootstrap_symbol << "\n";
    out << "ea_wakeup=" << b.ea_wakeup << "\n";
    out << "system_bandwidth=" << b.system_bandwidth << "\n";
    out << "bsr_coefficient=" << b.bsr_coefficient << "\n";
    out << "min_time_to_next=" << b.min_time_to_next << "\n";
    out << "preamble_structure=" << b.preamble_structure << "\n";
    out << "number_of_frames=" << b.number_of_frames << "\n";

    out << "# =============================================\n";
    out << "# PREAMBLE CONFIGURATION\n";
    out << "# =============================================\n";
    out << "L1B_version=" << p.l1b_version << "\n";
    out << "L1B_mimo_scattered_pilot_encoding=" << p.l1b_mimo_scattered_pilot_encoding << "\n";
    out << "L1B_lls_flag=" << p.l1b_lls_flag << "\n";
    out << "L1B_time_info_flag=" << p.l1b_time_info_flag << "\n";
    out << "L1B_return_channel_flag=" << p.l1b_return_channel_flag << "\n";
    out << "L1B_papr_reduction=" << p.l1b_papr_reduction << "\n";
    out << "L1B_frame_length_mode=" << p.l1b_frame_length_mode << "\n";
    out << "L1B_frame_length=" << p.l1b_frame_length << "\n";
    out << "L1B_excess_samples_per_symbol=" << p.l1b_excess_samples_per_symbol << "\n";
    out << "L1B_time_offset=" << p.l1b_time_offset << "\n";
    out << "L1B_additional_samples=" << p.l1b_additional_samples << "\n";
    out << "L1B_num_subframes=" << p.l1b_num_subframes << "\n";
    out << "L1B_preamble_num_symbols=" << p.l1b_preamble_num_symbols << "\n";
    out << "L1B_preamble_reduced_carriers=" << p.l1b_preamble_reduced_carriers << "\n";
    out << "L1B_L1_Detail_content_tag=" << p.l1b_detail_content_tag << "\n";
    out << "L1B_L1_Detail_size_bytes=" << std::max(p.l1b_detail_size_bytes, 25) << "\n";
    out << "L1B_L1_Detail_fec_type=" << p.l1b_detail_fec_type << "\n";
    out << "L1B_L1_Detail_additional_parity_mode=" << p.l1b_detail_additional_parity_mode << "\n";
    out << "L1B_L1_Detail_total_cells=" << p.l1b_detail_total_cells << "\n";
    out << "L1D_version=" << p.l1d_version << "\n";
    out << "L1D_bsid=" << p.l1d_bsid << "\n";
    out << "L1D_num_rf=" << p.l1d_num_rf << "\n";
    out << "L1D_rf_id=" << p.l1d_rf_id << "\n";
    out << "L1D_bonded_bsid=" << p.l1d_bonded_bsid << "\n";
    out << "L1D_time_sec=" << p.l1d_time_sec << "\n";
    out << "L1D_time_msec=" << p.l1d_time_msec << "\n";
    out << "L1D_time_usec=" << p.l1d_time_usec << "\n";
    out << "L1D_time_nsec=" << p.l1d_time_nsec << "\n";

    if (!config.subframes.empty()) {
        out << "# =============================================\n";
        out << "# SUBFRAMES & PLPs CONFIGURATION\n";
        out << "# =============================================\n";

        for (int i = 0; i < (int)config.subframes.size(); i++) {
            const auto& sf = config.subframes[i];
            if (i == 0) {
                out << "\n# --- Subframe Basic " << i << " ---\n";
                out << "subframe.[" << i << "].L1B_first_sub_mimo=" << sf.mimo << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_mimo_mixed=" << sf.mimo_mixed << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_miso=" << sf.miso << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_fft_size=" << sf.fft_size << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_reduced_carriers=" << sf.reduced_carriers << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_guard_interval=" << sf.guard_interval << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_num_ofdm_symbols=" << sf.num_ofdm_symbols << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_scattered_pilot_pattern=" << sf.scattered_pilot_pattern << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_scattered_pilot_boost=" << sf.scattered_pilot_boost << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_sbs_first=" << sf.sbs_first << "\n";
                out << "subframe.[" << i << "].L1B_first_sub_sbs_last=" << sf.sbs_last << "\n";
            } else {
                out << "\n# --- Subframe Detail " << i << " ---\n";
                out << "subframe.[" << i << "].L1D_mimo=" << sf.mimo << "\n";
                out << "subframe.[" << i << "].L1D_mimo_mixed=" << sf.mimo_mixed << "\n";
                out << "subframe.[" << i << "].L1D_miso=" << sf.miso << "\n";
                out << "subframe.[" << i << "].L1D_fft_size=" << sf.fft_size << "\n";
                out << "subframe.[" << i << "].L1D_reduced_carriers=" << sf.reduced_carriers << "\n";
                out << "subframe.[" << i << "].L1D_guard_interval=" << sf.guard_interval << "\n";
                out << "subframe.[" << i << "].L1D_num_ofdm_symbols=" << sf.num_ofdm_symbols << "\n";
                out << "subframe.[" << i << "].L1D_scattered_pilot_pattern=" << sf.scattered_pilot_pattern << "\n";
                out << "subframe.[" << i << "].L1D_scattered_pilot_boost=" << sf.scattered_pilot_boost << "\n";
                out << "subframe.[" << i << "].L1D_sbs_first=" << sf.sbs_first << "\n";
                out << "subframe.[" << i << "].L1D_sbs_last=" << sf.sbs_last << "\n";
            }
            out << "subframe.[" << i << "].L1D_subframe_multiplex=" << sf.subframe_multiplex << "\n";
            out << "subframe.[" << i << "].L1D_frequency_interleaver=" << sf.frequency_interleaver << "\n";
            out << "subframe.[" << i << "].L1D_sbs_null_cells=" << sf.sbs_null_cells << "\n";
            out << "subframe.[" << i << "].L1D_num_plp=" << (int)sf.plps.size() << "\n";

            for (int j = 0; j < (int)sf.plps.size(); j++) {
                const auto& plp = sf.plps[j];
                out << "\n# Subframe " << i << " - PLP " << j << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_id=" << plp.id << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_lls_flag=" << plp.lls_flag << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_layer=" << plp.layer << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_start=" << plp.start << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_size=" << plp.size << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_scrambler_type=" << plp.scrambler_type << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_fec_type=" << plp.fec_type << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mod=" << plp.mod << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_cod=" << plp.cod << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_TI_mode=" << plp.ti_mode << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_fec_block_start=" << plp.fec_block_start << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_CTI_fec_block_start=" << plp.cti_fec_block_start << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_num_channel_bonded=" << plp.num_channel_bonded << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_channel_bonding_format=" << plp.channel_bonding_format << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_bonded_rf_id=" << plp.bonded_rf_id << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_TI_extended_interleaving=" << plp.ti_extended_interleaving << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_CTI_depth=" << plp.cti_depth << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_CTI_start_row=" << plp.cti_start_row << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo=" << plp.mimo << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo_stream_combining=" << plp.mimo_stream_combining << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo_IQ_interleaving=" << plp.mimo_iq_interleaving << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_mimo_PH=" << plp.mimo_ph << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_type=" << plp.type << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_num_subslices=" << plp.num_subslices << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_subslice_interval=" << plp.subslice_interval << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_inter_subframe=" << plp.hti_inter_subframe << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_num_ti_blocks=" << plp.hti_num_ti_blocks << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_num_fec_blocks_max=" << plp.hti_num_fec_blocks_max << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_num_fec_blocks=" << plp.hti_num_fec_blocks << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_HTI_cell_interleaver=" << plp.hti_cell_interleaver << "\n";
                out << "subframe.[" << i << "].plp.[" << j << "].L1D_plp_ldm_injection_level=" << plp.ldm_injection_level << "\n";
            }
        }
    }

    out << "\n# =============================================\n";
    out << "# END OF CONFIGURATION FILE\n";
    out << "# =============================================\n";

    file.close();
    return true;
}

}
