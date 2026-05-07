// =============================================================================
// Teste end-to-end: setConfig -> computeResultsData
//
// Fluxo identico ao da pagina real:
//   1. webpage.php POST  -> ConfigManager::buildJsonPayload  -> setConfig
//   2. atsc-calculator.js -> fetchResultsData                -> computeResultsData
//
// Cada SCENARIO instancia um Bench limpo (ATSC_Config + L1_Calculator +
// L1_Generator + L1_Interpreter + JsonHandler) e dispara as duas chamadas
// na ordem em que a pagina as faria.
//
// Os valores expected sao snapshots capturados da ferramenta de referencia
// (resposta JSON real do api.php / log Frame_2.log) para o cenario T1.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "json_handler.h"
#include "atsc_config.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "nlohmann/json.hpp"

#include <filesystem>

using nlohmann::json;
using Catch::Matchers::WithinAbs;

namespace {

struct Bench {
    ATSC_Config    config{};
    L1_Calculator  calc{config};
    L1_Generator   gen{config, calc};
    L1_Interpreter interp{config};
    JsonHandler    handler{config, calc, gen, interp};

    json call(const json& req) {
        return json::parse(handler.processRequest(req.dump()));
    }
};

// -------------------------------------------------------------------------
// T1 — 1 subframe, 16K-QPSK-CR=9/15, 64K LDPC, 1 PLP de 632933 cells
//
// Configuracao reconstituida a partir do Frame_2.log da ferramenta de
// referencia:
//   L1B_first_sub_fft_size           = 16K           -> fft_size: 1
//   L1B_first_sub_guard_interval     = GI5_1024      -> guard_interval: 5
//   L1B_first_sub_num_ofdm_symbols   = 50            -> num_ofdm: 50
//   L1B_first_sub_scattered_pilot    = SP6_2         -> spilot_pattern: 4
//   L1B_first_sub_scattered_pilot_b. = 0.00 dB       -> spilot_boost: 0
//   L1B_first_sub_sbs_first          = NOT boundary  -> sbs_first: 0
//   L1B_first_sub_sbs_last           = NOT boundary  -> sbs_last: 0
//   L1B_L1_Detail_fec_type           = Mode 1        -> detail_fec_type: 0
//   L1D_plp_size                     = 632933
//   L1D_plp_fec_type                 = BCH + 64K     -> fec_type: 1
//   L1D_plp_mod                      = QPSK          -> mod_order: 0
//   L1D_plp_cod                      = 9/15          -> code_rate: 7
//   L1D_plp_TI_mode                  = no TI         -> ti_mode: 0
// -------------------------------------------------------------------------
json buildSetConfigPayload_T1() {
    return {
        {"flag", "setConfig"},

        // Bootstrap
        {"major_version",    0},   // -> 137
        {"minor_version",    0},
        {"bootstrap_symbol", 0},   // -> 4
        {"ea_wakeup",        0},
        {"system_bandwidth", 0},   // -> 6 MHz
        {"bsr_coefficient",  2},   // Fs = 384000 * (2 + 16) = 6.912 MHz
                                   // (ver l1_calculator.cpp:767)
        {"min_time_to_next", 0},
        {"preamble_structure", 5},

        // Frame
        {"number_of_frames",   10},  // referencia gera Frame_10.log
        {"frame_lenght_mode",  1},   // 1 = symbol-aligned, no excess
                                     // (0 = time-aligned, ver dicionary_basic.h:83-85)
        {"frame_lenght",       250},
        {"time_info_flag",     0},

        // Preamble L1B/L1D
        {"l1b_version",                        0},
        {"l1b_mimo_scatterred_pilot_encoding", 0},
        {"l1d_version",                        0},
        {"l1d_bsid",                           1},
        {"detail_fec_type",                    0},

        // Subframes
        {"number_of_subframes", 1},
        {"subframes", json::array({{
            {"plp_mimo",        0},
            {"plp_mimo_mixed",  0},
            {"plp_miso",        0},
            {"fft_size",        1},   // 16K
            {"reduced_carrier", 0},
            {"guard_interval",  5},   // GI5_1024
            {"num_ofdm",        50},
            {"spilot_pattern",  4},   // SP6_2
            {"spilot_boost",    0},
            {"sbs_first",       0},
            {"sbs_last",        0},
            {"freq_interleaver", 0},
            {"plps", json::array({{
                {"plp_id",      0},
                {"lls_flag",    0},
                {"layer",       0},
                {"start",       0},
                {"size",        632933},   // auto-fill
                {"fec_type",    1},        // 64K LDPC
                {"mod_order",   0},        // QPSK
                {"code_rate",   7},        // 9/15
                {"ti_mode",     0},        // no TI
                {"ti_extended", 0},
                {"cti_depth",   0},
                {"plp_type",    0},
                {"num_subslice",       1},
                {"subslice_interval",  0},
                {"num_fec_blocks_max", 100},
                {"num_fec_blocks",     19},
                {"ldm_injection_level", 0}
            }})}
        }})}
    };
}

// Espelha exatamente o payload montado por
// PlpCalculator.fetchResultsData() em atsc-calculator.js para esse
// cenario na pagina.
json buildComputeResultsPayload_T1() {
    return {
        {"flag", "computeResultsData"},
        {"preambleConfig", {
            {"fftValue",                 1},   // 1 = 16K (igual ao subframe)
            {"giValue",                  5},
            {"preambleReducedCarriers",  0},
            {"preambleStructure",        5},
            {"l1dFecMode",               0},
            {"l1dBytes",                 25}
        }},
        {"subframesData", json::array({{
            {"fft",           "16K"},
            {"cred",          0},
            {"spLabelText",   "4"},     // value=4 -> SP6_2 via SP_PATTERN_MAP
            {"spBoost",       0},
            {"numSymbols",    50},
            {"sbsFirst",      false},
            {"sbsLast",       false},
            {"subframeIndex", 0},
            {"plps", json::array({{
                {"fec_type",           1},
                {"mod_order",          0},
                {"code_rate",          7},
                {"size",               632933},
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

} // namespace


SCENARIO("T1: 1 subframe 16K-QPSK-CR9/15 64K-LDPC, 1 PLP de 632933 cells") {

    GIVEN("Working dir com config/log preparado e Bench limpo") {
        std::filesystem::create_directories("config/log");
        Bench b;

        WHEN("Pagina submete o form (setConfig) e em seguida pede os resultados (computeResultsData)") {
            auto setRes = b.call(buildSetConfigPayload_T1());
            REQUIRE(setRes["status"]   == "ok");
            REQUIRE(setRes["mensagem"] == "Configuracao aplicada com sucesso");

            auto res = b.call(buildComputeResultsPayload_T1());

            THEN("Status global e ok e blocos esperados estao presentes") {
                REQUIRE(res["status"] == "ok");
                REQUIRE(res.contains("frameDuration"));
                REQUIRE(res.contains("frame2LogData"));
                REQUIRE(res.contains("l1dSizeBytes"));
                REQUIRE(res.contains("preambleFields"));
                REQUIRE(res.contains("subframeResults"));
                REQUIRE(res["subframeResults"].is_array());
                REQUIRE(res["subframeResults"].size() == 1);
            }

            // -----------------------------------------------------------------
            // Frame duration
            // -----------------------------------------------------------------
            THEN("Frame duration bate com a ferramenta de referencia") {
                auto& fd = res["frameDuration"];
                REQUIRE(fd["status"] == "ok");

                CHECK_THAT(fd["totalDurationMs"].get<double>(),
                           WithinAbs(130.444444, 0.0001));

                REQUIRE(fd["subframeDurations"].contains("0"));
                CHECK_THAT(fd["subframeDurations"]["0"].get<double>(),
                           WithinAbs(125.925926, 0.0001));
            }

            // -----------------------------------------------------------------
            // Frame_2.log: timeOffset e fecBlockStarts
            // -----------------------------------------------------------------
            THEN("Frame_2.log: time offset e fec block start batem com a referencia") {
                auto& fl = res["frame2LogData"];
                REQUIRE(fl["status"] == "ok");
                CHECK(fl["source"]   == "Frame_2.log");

                CHECK(fl["timeOffset"].get<int>() == 3072);

                REQUIRE(fl["fecBlockStarts"].is_array());
                REQUIRE(fl["fecBlockStarts"].size() == 1);
                auto& plp0 = fl["fecBlockStarts"][0];
                CHECK(plp0["plpId"].get<int>()         == 0);
                CHECK(plp0["fecBlockStart"].get<int>() == 15067);
            }

            // -----------------------------------------------------------------
            // L1D size bytes (lido de Frame_10.log)
            // -----------------------------------------------------------------
            THEN("L1D size bytes bate com a referencia") {
                auto& l1 = res["l1dSizeBytes"];
                REQUIRE(l1["status"] == "ok");
                CHECK(l1["bytes"].get<int>() == 25);

                // handleGetL1dSizeBytes seleciona o Frame_*.log com mtime mais
                // recente. Em prod a pagina ve "Frame_10.log" (ultimo gerado),
                // mas no teste os 10 arquivos sao escritos em fracao de ms e
                // o mtime tende a empatar — desempate cai no primeiro da
                // iteracao do diretorio (nao-deterministico). So validamos o
                // padrao do nome.
                std::string source = l1["source"].get<std::string>();
                CHECK(source.rfind("Frame_", 0) == 0);
                CHECK(source.find(".log") != std::string::npos);
            }

            // -----------------------------------------------------------------
            // Preamble fields
            // -----------------------------------------------------------------
            THEN("Preamble fields batem com a referencia") {
                auto& p = res["preambleFields"];
                CHECK(p["status"]                  == "ok");
                CHECK(p["fftName"]                 == "16K");
                CHECK(p["giSamples"]               == 1024);
                CHECK(p["numSymbols"]              == 1);
                CHECK(p["preambleDx"]              == 0);
                CHECK(p["preambleReducedCarriers"] == 0);
                CHECK(p["l1bCells"]                == 3820);
                CHECK(p["l1dCells"]                == 2787);
                CHECK(p["l1dBytes"]                == 25);
                CHECK(p["cellsInFirst"]            == 10790);
                CHECK(p["cellsInNext"]             == 11424);
                CHECK(p["plpCells"]                == 4183);
                CHECK(p["totalPreambleCells"]      == 10790);
            }

            // -----------------------------------------------------------------
            // PLP capacity
            //
            // Capacidade total para o PLP = capacity (subframe) + plpCells
            // (sobra do simbolo de preambulo) = 628750 + 4183 = 632933
            // -----------------------------------------------------------------
            THEN("Capacity do subframe bate com a referencia") {
                auto& cap = res["subframeResults"][0]["plpCapacity"];
                CHECK(cap["status"]                 == "ok");
                CHECK(cap["fft"]                    == "16K");
                CHECK(cap["cred"]                   == 0);
                CHECK(cap["spBoost"]                == 0);
                CHECK(cap["nDataSymbols"]           == 50);
                CHECK(cap["nSbsSymbols"]            == 0);
                CHECK(cap["cps"].get<int>()         == 12575);
                CHECK(cap["sbsDataCells"].get<int>() == 11424);
                CHECK(cap["capacity"].get<int>()    == 628750);
                CHECK(cap["capacitySBS"].get<int>() == 0);

                // Identidade: capacity + plpCells (do preambulo) = PLP size
                int totalAvailable =
                    cap["capacity"].get<int>() +
                    res["preambleFields"]["plpCells"].get<int>();
                CHECK(totalAvailable == 632933);
            }

            // -----------------------------------------------------------------
            // PLP[0] — FEC, modulacao, bitrate
            // -----------------------------------------------------------------
            THEN("PLP[0]: identidades de FEC/modulacao batem com a referencia") {
                auto& plp = res["subframeResults"][0]["plpResults"][0];

                CHECK(plp["ldpcSize"]     == 64800);
                CHECK(plp["bitsPerCell"]  == 2);
                CHECK(plp["fecBlockSize"] == 32400);   // 64800 / 2
                CHECK(plp["numFecBlocks"] == 19);      // 632933 / 32400

                CHECK(plp["celulasTI"].get<int>() == 130816);  // ti_mode=0

                CHECK(plp["bbFrameRate"].get<int>() == 150);

                CHECK_THAT(plp["bitrateMbps"].get<double>(),
                           WithinAbs(5.791, 0.0005));
            }

            // -----------------------------------------------------------------
            // CNR (BICM/Simulation/Lab/Field) — QPSK-CR9/15, 64K LDPC
            // -----------------------------------------------------------------
            THEN("CNR PLP[0] bate com a referencia") {
                auto& cnr = res["subframeResults"][0]["plpResults"][0]["cnr"];
                REQUIRE(cnr.is_object());

                CHECK_THAT(cnr["bicm"]["awgn"].get<double>(),
                           WithinAbs(1.97, 0.005));

                CHECK_THAT(cnr["simulation"]["awgn"].get<double>(),
                           WithinAbs(2.10, 0.005));
                CHECK_THAT(cnr["simulation"]["rc20"].get<double>(),
                           WithinAbs(2.50, 0.005));
                CHECK_THAT(cnr["simulation"]["rl20"].get<double>(),
                           WithinAbs(4.30, 0.005));

                CHECK_THAT(cnr["lab"]["awgn"].get<double>(),
                           WithinAbs(2.80, 0.005));
                CHECK_THAT(cnr["lab"]["rc20"].get<double>(),
                           WithinAbs(3.20, 0.005));
                CHECK_THAT(cnr["lab"]["rl20"].get<double>(),
                           WithinAbs(5.60, 0.005));

                CHECK_THAT(cnr["field"]["awgn"].get<double>(),
                           WithinAbs(2.90, 0.005));
                CHECK_THAT(cnr["field"]["rc20"].get<double>(),
                           WithinAbs(4.20, 0.005));
                CHECK_THAT(cnr["field"]["rl20"].get<double>(),
                           WithinAbs(4.60, 0.005));
            }
        }
    }
}


// =============================================================================
// Tabela de cenarios — preencher conforme avancar a validacao
// =============================================================================
//
// struct Scenario {
//     std::string name;
//     int    fftSize;        // 0=8K, 1=16K, 2=32K
//     int    guardInterval;  // 1..12
//     int    numOfdm;
//     int    modOrder;       // 0..5
//     int    codeRate;       // 0..11
//     int    fecType;        // 0=16K, 1=64K
//     int    plpSize;
//     // expected (capturados da ferramenta de referencia):
//     double expectedFrameDurationMs;
//     int    expectedNumSymbols;
//     int    expectedFecBlockSize;
//     double expectedBitrateMbps;
// };
//
// static const std::vector<Scenario> SCENARIOS = {
//     // {"T2_16K_16QAM_CR8_15", 1, 5, 200, 1, 6, 1, 80000, /*…*/},
//     // {"T3_32K_64QAM_CR9_15", 2, 7, 300, 2, 7, 1, /*…*/},
// };
