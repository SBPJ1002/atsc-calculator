// =============================================================================
// Helpers compartilhados entre os arquivos de teste:
//   - struct Bench (cria ATSC_Config + L1_Calculator + Generator + Interpreter
//     + JsonHandler todos linkados juntos)
//   - SisoScenario, SISO_VALID (Table 8.3 SISO), SISO_INVALID, MIMO_ENCODINGS
//   - buildSetConfig / buildSetConfigMimo / buildComputeResults
//   - fftLabel / spLabel / giLabel / scenarioName
//
// Tudo `inline` para evitar ODR violations quando incluido em multiplos
// translation units.
// =============================================================================

#ifndef SISO_SCENARIOS_H
#define SISO_SCENARIOS_H

#include "atsc_config.h"
#include "json_handler.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "nlohmann/json.hpp"

#include <string>
#include <vector>

namespace siso {

using json = nlohmann::json;

struct Bench {
    ATSC_Config    config{"/tmp/test_siso_scenarios_unused.conf"};
    L1_Calculator  calc{config};
    L1_Generator   gen{config, calc};
    L1_Interpreter interp{config};
    JsonHandler    handler{config, calc, gen, interp};

    json call(const json& req) {
        return json::parse(handler.processRequest(req.dump()));
    }
};

inline const char* fftLabel(int fftSize) {
    switch (fftSize) {
        case 0: return "8K";
        case 1: return "16K";
        case 2: return "32K";
        default: return "8K";
    }
}

inline const char* spLabel(int code) {
    static const char* names[16] = {
        "SP3_2","SP3_4","SP4_2","SP4_4","SP6_2","SP6_4","SP8_2","SP8_4",
        "SP12_2","SP12_4","SP16_2","SP16_4","SP24_2","SP24_4","SP32_2","SP32_4"
    };
    return (code >= 0 && code < 16) ? names[code] : "?";
}

inline const char* giLabel(int gi) {
    static const char* names[13] = {
        "GI0", "GI1_192","GI2_384","GI3_512","GI4_768","GI5_1024","GI6_1536",
        "GI7_2048","GI8_2432","GI9_3072","GI10_3648","GI11_4096","GI12_4864"
    };
    return (gi >= 0 && gi < 13) ? names[gi] : "?";
}

struct SisoScenario {
    int fft;     // 0=8K, 1=16K, 2=32K
    int gi;      // 1..12
    int sp;      // 0..15 (codigo SP_PATTERN_MAP)
};

// Table 8.3 — Allowed Scattered Pilot Pattern for Each Combination of FFT
// Size and Guard Interval Pattern in SISO Mode (A/322 Section 8). 81 combos.
inline const std::vector<SisoScenario> SISO_VALID = {
    // 8K
    {0,1,14},{0,1,15},{0,1,10},{0,1,11},
    {0,2,10},{0,2,11},{0,2,6},{0,2,7},
    {0,3,8},{0,3,9},{0,3,4},{0,3,5},
    {0,4,6},{0,4,7},{0,4,2},{0,4,3},
    {0,5,4},{0,5,5},{0,5,0},{0,5,1},
    {0,6,2},{0,6,3},
    {0,7,0},{0,7,1},
    // 16K
    {1,1,14},{1,1,15},
    {1,2,14},{1,2,15},{1,2,10},{1,2,11},
    {1,3,12},{1,3,13},{1,3,8},{1,3,9},
    {1,4,10},{1,4,11},{1,4,6},{1,4,7},
    {1,5,8},{1,5,9},{1,5,4},{1,5,5},
    {1,6,6},{1,6,7},{1,6,2},{1,6,3},
    {1,7,4},{1,7,5},{1,7,0},{1,7,1},
    {1,8,4},{1,8,5},{1,8,0},{1,8,1},
    {1,9,2},{1,9,3},
    {1,10,2},{1,10,3},
    {1,11,0},{1,11,1},
    // 32K
    {2,1,14},
    {2,2,14},
    {2,3,12},
    {2,4,14},{2,4,10},
    {2,5,12},{2,5,8},
    {2,6,10},{2,6,6},
    {2,7,8},{2,7,4},
    {2,8,8},{2,8,4},
    {2,9,6},{2,9,0},
    {2,10,6},{2,10,0},
    {2,11,4},{2,11,0},
    {2,12,4},{2,12,0},
};

// Casos N/A na Table 8.3.
inline const std::vector<SisoScenario> SISO_INVALID = {
    {0,8,0}, {0,9,0}, {0,10,0}, {0,11,0}, {0,12,0},
    {1,12,0},
    {2,1,4},
    {2,3,4},
};

struct MimoEncoding {
    int code;            // 0 = Walsh-Hadamard, 1 = Null-pilot
    const char* name;
};

inline const std::vector<MimoEncoding> MIMO_ENCODINGS = {
    {0, "Walsh-Hadamard"},
    {1, "Null-pilot"},
};

inline std::string scenarioName(const SisoScenario& s) {
    return std::string(fftLabel(s.fft)) + " + " + giLabel(s.gi) + " + " + spLabel(s.sp);
}

// Constroi um payload setConfig completo para o cenario com defaults sanos
// para todos os outros campos. Modulacao/code rate fixos (QPSK 9/15 com 64K
// LDPC) — esses nao influenciam Table 8.3.
inline json buildSetConfig(const SisoScenario& s) {
    return {
        {"flag", "setConfig"},

        {"major_version",     0},
        {"minor_version",     0},
        {"bootstrap_symbol",  0},
        {"ea_wakeup",         0},
        {"system_bandwidth",  0},
        {"bsr_coefficient",   2},
        {"min_time_to_next",  0},
        {"preamble_structure",5},

        {"number_of_frames",   1},
        {"frame_lenght_mode",  1},
        {"frame_lenght",       250},
        {"time_info_flag",     0},

        {"l1b_version",                        0},
        {"l1b_mimo_scatterred_pilot_encoding", 0},
        {"l1d_version",                        0},
        {"l1d_bsid",                           1},
        {"detail_fec_type",                    0},

        {"number_of_subframes", 1},
        {"subframes", json::array({{
            {"plp_mimo",        0},
            {"plp_mimo_mixed",  0},
            {"plp_miso",        0},
            {"fft_size",        s.fft},
            {"reduced_carrier", 0},
            {"guard_interval",  s.gi},
            {"num_ofdm",        30},
            {"spilot_pattern",  s.sp},
            {"spilot_boost",    0},
            {"sbs_first",       0},
            {"sbs_last",        0},
            {"freq_interleaver", 0},
            {"plps", json::array({{
                {"plp_id",      0},
                {"lls_flag",    0},
                {"layer",       0},
                {"start",       0},
                {"size",        100},
                {"fec_type",    1},
                {"mod_order",   0},
                {"code_rate",   7},
                {"ti_mode",     0},
                {"ti_extended", 0},
                {"cti_depth",   0},
                {"plp_type",    0},
                {"num_subslice",       1},
                {"subslice_interval",  0},
                {"num_fec_blocks_max", 100},
                {"num_fec_blocks",     1},
                {"ldm_injection_level", 0}
            }})}
        }})}
    };
}

inline json buildSetConfigMimo(const SisoScenario& s, int encoding) {
    json req = buildSetConfig(s);
    req["l1b_mimo_scatterred_pilot_encoding"] = encoding;
    auto& sub = req["subframes"][0];
    sub["plp_mimo"] = 1;
    sub["plp_mimo_mixed"] = 0;
    sub["plp_miso"] = 0;
    sub["plps"][0]["plp_mimo"] = 1;
    sub["plps"][0]["stream_combining"] = 0;
    sub["plps"][0]["iq_intervaling"] = 0;
    sub["plps"][0]["phase_hopping"] = 0;
    return req;
}

inline json buildComputeResults(const SisoScenario& s) {
    return {
        {"flag", "computeResultsData"},
        {"preambleConfig", {
            {"fftValue",                 s.fft},
            {"giValue",                  s.gi},
            {"preambleReducedCarriers",  0},
            {"preambleStructure",        5},
            {"l1dFecMode",               0},
            {"l1dBytes",                 25}
        }},
        {"subframesData", json::array({{
            {"fft",           fftLabel(s.fft)},
            {"cred",          0},
            {"spLabelText",   std::to_string(s.sp)},
            {"spBoost",       0},
            {"numSymbols",    30},
            {"sbsFirst",      false},
            {"sbsLast",       false},
            {"subframeIndex", 0},
            {"plps", json::array({{
                {"fec_type",           1},
                {"mod_order",          0},
                {"code_rate",          7},
                {"size",               100},
                {"start",              0},
                {"ti_mode",            0},
                {"ti_extended",        0},
                {"cti_depth",          0},
                {"num_fec_blocks_max", 100},
                {"plp_id",             0}
            }})}
        }})}
    };
}

} // namespace siso

#endif
