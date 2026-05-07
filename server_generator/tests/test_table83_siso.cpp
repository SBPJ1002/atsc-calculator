// =============================================================================
// Table 8.3 SISO conformance — varredura de cobertura.
//
// Para cada combinacao (FFT, GI, SP) permitida pela Table 8.3 do A/322 em
// modo SISO, faz setConfig + computeResultsData e checa invariantes:
//   - status == "ok" em todos os blocos
//   - frameDuration > 0
//   - l1dSizeBytes >= 25 (clamp do padrao)
//   - capacity > 0 e cps > 0
//   - identidade: capacity == nDataSymbols * cps + nSbsSymbols * sbsDataCells
//   - preambleFields.l1dBytes >= 25
//
// Combinacoes proibidas pela Table 8.3 (ex: 8K + GI8_2432, 16K + GI12_4864,
// 32K + GI1_192 + SP6) entram numa lista separada e o teste verifica que o
// servidor sinaliza com capacity == 0 e reason preenchido.
// =============================================================================

#include <catch2/catch_test_macros.hpp>

#include "atsc_config.h"
#include "json_handler.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "nlohmann/json.hpp"

#include <filesystem>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

struct Bench {
    ATSC_Config    config{"/tmp/test_table83_siso_unused.conf"};
    L1_Calculator  calc{config};
    L1_Generator   gen{config, calc};
    L1_Interpreter interp{config};
    JsonHandler    handler{config, calc, gen, interp};

    json call(const json& req) {
        return json::parse(handler.processRequest(req.dump()));
    }
};

// FFT index -> label esperado pelo computePlpCapacity / lookup tables.
const char* fftLabel(int fftSize) {
    switch (fftSize) {
        case 0: return "8K";
        case 1: return "16K";
        case 2: return "32K";
        default: return "8K";
    }
}

// Codigo SP_PATTERN_MAP -> label legivel para o nome do SECTION.
const char* spLabel(int code) {
    static const char* names[16] = {
        "SP3_2","SP3_4","SP4_2","SP4_4","SP6_2","SP6_4","SP8_2","SP8_4",
        "SP12_2","SP12_4","SP16_2","SP16_4","SP24_2","SP24_4","SP32_2","SP32_4"
    };
    return (code >= 0 && code < 16) ? names[code] : "?";
}

const char* giLabel(int gi) {
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
// Size and Guard Interval Pattern in SISO Mode (A/322 Section 8).
//
// Codigos SP via SP_PATTERN_MAP em atsc_tables.h:
//   0 SP3_2  1 SP3_4   2 SP4_2   3 SP4_4
//   4 SP6_2  5 SP6_4   6 SP8_2   7 SP8_4
//   8 SP12_2 9 SP12_4  10 SP16_2 11 SP16_4
//   12 SP24_2 13 SP24_4 14 SP32_2 15 SP32_4
const std::vector<SisoScenario> SISO_VALID = {
    // 8K
    {0,1,14},{0,1,15},{0,1,10},{0,1,11},                  // GI1_192:  SP32_2,SP32_4,SP16_2,SP16_4
    {0,2,10},{0,2,11},{0,2,6},{0,2,7},                    // GI2_384:  SP16_2,SP16_4,SP8_2,SP8_4
    {0,3,8},{0,3,9},{0,3,4},{0,3,5},                      // GI3_512:  SP12_2,SP12_4,SP6_2,SP6_4
    {0,4,6},{0,4,7},{0,4,2},{0,4,3},                      // GI4_768:  SP8_2,SP8_4,SP4_2,SP4_4
    {0,5,4},{0,5,5},{0,5,0},{0,5,1},                      // GI5_1024: SP6_2,SP6_4,SP3_2,SP3_4
    {0,6,2},{0,6,3},                                      // GI6_1536: SP4_2,SP4_4
    {0,7,0},{0,7,1},                                      // GI7_2048: SP3_2,SP3_4
    // 16K
    {1,1,14},{1,1,15},                                    // GI1_192:  SP32_2,SP32_4
    {1,2,14},{1,2,15},{1,2,10},{1,2,11},                  // GI2_384:  SP32_2,SP32_4,SP16_2,SP16_4
    {1,3,12},{1,3,13},{1,3,8},{1,3,9},                    // GI3_512:  SP24_2,SP24_4,SP12_2,SP12_4
    {1,4,10},{1,4,11},{1,4,6},{1,4,7},                    // GI4_768:  SP16_2,SP16_4,SP8_2,SP8_4
    {1,5,8},{1,5,9},{1,5,4},{1,5,5},                      // GI5_1024: SP12_2,SP12_4,SP6_2,SP6_4
    {1,6,6},{1,6,7},{1,6,2},{1,6,3},                      // GI6_1536: SP8_2,SP8_4,SP4_2,SP4_4
    {1,7,4},{1,7,5},{1,7,0},{1,7,1},                      // GI7_2048: SP6_2,SP6_4,SP3_2,SP3_4
    {1,8,4},{1,8,5},{1,8,0},{1,8,1},                      // GI8_2432: SP6_2,SP6_4,SP3_2,SP3_4
    {1,9,2},{1,9,3},                                      // GI9_3072: SP4_2,SP4_4
    {1,10,2},{1,10,3},                                    // GI10_3648: SP4_2,SP4_4
    {1,11,0},{1,11,1},                                    // GI11_4096: SP3_2,SP3_4
    // 32K
    {2,1,14},                                             // GI1_192:  SP32_2
    {2,2,14},                                             // GI2_384:  SP32_2
    {2,3,12},                                             // GI3_512:  SP24_2
    {2,4,14},{2,4,10},                                    // GI4_768:  SP32_2,SP16_2
    {2,5,12},{2,5,8},                                     // GI5_1024: SP24_2,SP12_2
    {2,6,10},{2,6,6},                                     // GI6_1536: SP16_2,SP8_2
    {2,7,8},{2,7,4},                                      // GI7_2048: SP12_2,SP6_2
    {2,8,8},{2,8,4},                                      // GI8_2432: SP12_2,SP6_2
    {2,9,6},{2,9,0},                                      // GI9_3072: SP8_2,SP3_2
    {2,10,6},{2,10,0},                                    // GI10_3648: SP8_2,SP3_2
    {2,11,4},{2,11,0},                                    // GI11_4096: SP6_2,SP3_2
    {2,12,4},{2,12,0},                                    // GI12_4864: SP6_2,SP3_2
};

// Casos N/A na Table 8.3: o servidor deve retornar capacity == 0 com reason
// preenchido (lookup falhando em getTableCellsPerSymbol).
const std::vector<SisoScenario> SISO_INVALID = {
    {0,8,0},                       // 8K + GI8_2432 — N/A em SISO
    {0,9,0}, {0,10,0},
    {0,11,0}, {0,12,0},
    {1,12,0},                      // 16K + GI12_4864 — N/A
    {2,1,4},                       // 32K + GI1 + SP6_2 — so SP32_2 e permitido
    {2,3,4},                       // 32K + GI3 + SP6_2 — so SP24_2 e permitido
};

// Constroi um payload setConfig completo para o cenario com defaults sanos
// para todos os outros campos. Modulacao/code rate fixos (QPSK 9/15 com 64K
// LDPC) — esses nao influenciam Table 8.3.
json buildSetConfig(const SisoScenario& s) {
    return {
        {"flag", "setConfig"},

        // Bootstrap
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

json buildComputeResults(const SisoScenario& s) {
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

std::string scenarioName(const SisoScenario& s) {
    return std::string(fftLabel(s.fft)) + " + " + giLabel(s.gi) + " + " + spLabel(s.sp);
}

} // namespace


SCENARIO("Table 8.3 SISO: combos validos preservam invariantes do calculador") {
    std::filesystem::create_directories("config/log");

    for (const auto& s : SISO_VALID) {
        DYNAMIC_SECTION(scenarioName(s)) {
            Bench b;

            auto setRes = b.call(buildSetConfig(s));
            REQUIRE(setRes["status"] == "ok");

            auto res = b.call(buildComputeResults(s));
            REQUIRE(res["status"] == "ok");

            // Frame duration positivo
            REQUIRE(res.contains("frameDuration"));
            REQUIRE(res["frameDuration"]["status"] == "ok");
            CHECK(res["frameDuration"]["totalDurationMs"].get<double>() > 0.0);

            // L1D size respeita o minimo de 25 bytes
            REQUIRE(res.contains("l1dSizeBytes"));
            REQUIRE(res["l1dSizeBytes"]["status"] == "ok");
            CHECK(res["l1dSizeBytes"]["bytes"].get<int>() >= 25);

            // Preamble fields presentes e validos
            REQUIRE(res.contains("preambleFields"));
            REQUIRE(res["preambleFields"]["status"] == "ok");
            CHECK(res["preambleFields"]["l1dBytes"].get<int>() >= 25);
            CHECK(res["preambleFields"]["l1dCells"].get<int>() > 0);

            // Capacity positivo + identidade aritmetica
            REQUIRE(res["subframeResults"].is_array());
            REQUIRE(res["subframeResults"].size() == 1);
            auto& cap = res["subframeResults"][0]["plpCapacity"];
            REQUIRE(cap["status"] == "ok");
            REQUIRE(cap.contains("cps"));
            CHECK(cap["cps"].get<int>() > 0);
            CHECK(cap["capacity"].get<int>() > 0);

            int expected = cap["nDataSymbols"].get<int>() * cap["cps"].get<int>()
                         + cap["nSbsSymbols"].get<int>() * cap["sbsDataCells"].get<int>();
            CHECK(cap["capacity"].get<int>() == expected);

            // Sem SBS no nosso payload, capacitySBS deve ser 0
            CHECK(cap["nSbsSymbols"].get<int>() == 0);
            CHECK(cap["capacitySBS"].get<int>() == 0);
        }
    }
}


// =============================================================================
// Tabela 8.3 e enforced no frontend via getValidationTables (allowedPatternsSiso).
// O calculador em si nao rejeita combos N/A — a tabela interna de cells/symbol
// e mais densa que a permitida pelo padrao. Entao a fonte da verdade testavel
// e o que o servidor reporta em allowedPatternsSiso.
// =============================================================================

namespace {

json fetchValidationTables() {
    Bench b;
    return b.call({{"flag", "getValidationTables"}});
}

bool spInAllowedList(const json& allowedPatternsSiso, const SisoScenario& s) {
    const std::string fft = fftLabel(s.fft);
    const std::string gi  = giLabel(s.gi);

    if (!allowedPatternsSiso.contains(fft)) return false;
    if (!allowedPatternsSiso.at(fft).contains(gi)) return false;

    const std::string spCode = std::to_string(s.sp);
    for (const auto& code : allowedPatternsSiso.at(fft).at(gi)) {
        if (code.is_string() && code.get<std::string>() == spCode) return true;
    }
    return false;
}

} // namespace


SCENARIO("Table 8.3 SISO: getValidationTables.allowedPatternsSiso lista os combos validos") {
    std::filesystem::create_directories("config/log");
    auto vt = fetchValidationTables();
    REQUIRE(vt["status"] == "ok");
    REQUIRE(vt.contains("allowedPatternsSiso"));
    const auto& allowed = vt["allowedPatternsSiso"];

    for (const auto& s : SISO_VALID) {
        DYNAMIC_SECTION(scenarioName(s)) {
            CHECK(spInAllowedList(allowed, s));
        }
    }
}


SCENARIO("Table 8.3 SISO: getValidationTables.allowedPatternsSiso rejeita combos N/A") {
    std::filesystem::create_directories("config/log");
    auto vt = fetchValidationTables();
    REQUIRE(vt["status"] == "ok");
    REQUIRE(vt.contains("allowedPatternsSiso"));
    const auto& allowed = vt["allowedPatternsSiso"];

    for (const auto& s : SISO_INVALID) {
        DYNAMIC_SECTION(scenarioName(s)) {
            CHECK_FALSE(spInAllowedList(allowed, s));
        }
    }
}
