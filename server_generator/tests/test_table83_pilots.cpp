// =============================================================================
// Table 8.3 SISO + MIMO conformance — varredura de cobertura.
//
// Para cada combinacao (FFT, GI, SP) permitida pela Table 8.3 do A/322,
// faz setConfig + computeResultsData e checa invariantes:
//   - status == "ok" em todos os blocos
//   - frameDuration > 0
//   - l1dSizeBytes >= 25 (clamp do padrao)
//   - capacity > 0 e cps > 0
//   - identidade: capacity == nDataSymbols * cps + nSbsSymbols * sbsDataCells
//   - preambleFields.l1dBytes >= 25
//
// Combinacoes proibidas pela Table 8.3 sao verificadas no nivel da resposta
// de getValidationTables.allowedPatternsSiso.
//
// MIMO: re-roda os mesmos combos com plp_mimo=1 e os dois encodings
// (Walsh-Hadamard / Null-pilot), checando as mesmas invariantes + bitrate
// positivo.
// =============================================================================

#include <catch2/catch_test_macros.hpp>

#include "siso_scenarios.h"

#include <filesystem>
#include <string>

using siso::Bench;
using siso::SISO_VALID;
using siso::SISO_INVALID;
using siso::MIMO_ENCODINGS;
using siso::SisoScenario;
using siso::buildSetConfig;
using siso::buildSetConfigMimo;
using siso::buildComputeResults;
using siso::scenarioName;
using siso::fftLabel;
using siso::giLabel;
using siso::json;


SCENARIO("Table 8.3 SISO: combos validos preservam invariantes do calculador") {
    std::filesystem::create_directories("config/log");

    for (const auto& s : SISO_VALID) {
        DYNAMIC_SECTION(scenarioName(s)) {
            Bench b;

            auto setRes = b.call(buildSetConfig(s));
            REQUIRE(setRes["status"] == "ok");

            auto res = b.call(buildComputeResults(s));
            REQUIRE(res["status"] == "ok");

            REQUIRE(res.contains("frameDuration"));
            REQUIRE(res["frameDuration"]["status"] == "ok");
            CHECK(res["frameDuration"]["totalDurationMs"].get<double>() > 0.0);

            REQUIRE(res.contains("l1dSizeBytes"));
            REQUIRE(res["l1dSizeBytes"]["status"] == "ok");
            CHECK(res["l1dSizeBytes"]["bytes"].get<int>() >= 25);

            REQUIRE(res.contains("preambleFields"));
            REQUIRE(res["preambleFields"]["status"] == "ok");
            CHECK(res["preambleFields"]["l1dBytes"].get<int>() >= 25);
            CHECK(res["preambleFields"]["l1dCells"].get<int>() > 0);

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

            CHECK(cap["nSbsSymbols"].get<int>() == 0);
            CHECK(cap["capacitySBS"].get<int>() == 0);
        }
    }
}


SCENARIO("Varredura MIMO Walsh-Hadamard / Null-pilot: invariantes do calculador") {
    std::filesystem::create_directories("config/log");

    for (const auto& enc : MIMO_ENCODINGS) {
        for (const auto& s : SISO_VALID) {
            DYNAMIC_SECTION(std::string(enc.name) + " | " + scenarioName(s)) {
                Bench b;

                auto setRes = b.call(buildSetConfigMimo(s, enc.code));
                REQUIRE(setRes["status"] == "ok");

                auto res = b.call(buildComputeResults(s));
                REQUIRE(res["status"] == "ok");

                REQUIRE(res["frameDuration"]["status"] == "ok");
                CHECK(res["frameDuration"]["totalDurationMs"].get<double>() > 0.0);

                REQUIRE(res["l1dSizeBytes"]["status"] == "ok");
                CHECK(res["l1dSizeBytes"]["bytes"].get<int>() >= 25);

                REQUIRE(res["preambleFields"]["status"] == "ok");
                CHECK(res["preambleFields"]["l1dCells"].get<int>() > 0);

                auto& cap = res["subframeResults"][0]["plpCapacity"];
                REQUIRE(cap["status"] == "ok");
                REQUIRE(cap.contains("cps"));
                CHECK(cap["cps"].get<int>() > 0);
                CHECK(cap["capacity"].get<int>() > 0);

                int expected = cap["nDataSymbols"].get<int>() * cap["cps"].get<int>()
                             + cap["nSbsSymbols"].get<int>() * cap["sbsDataCells"].get<int>();
                CHECK(cap["capacity"].get<int>() == expected);

                auto& plp0 = res["subframeResults"][0]["plpResults"][0];
                CHECK(plp0["bitrateMbps"].get<double>() > 0.0);
                CHECK(plp0["ldpcSize"].get<int>() == 64800);
                CHECK(plp0["bitsPerCell"].get<int>()  == 2);
            }
        }
    }
}


// =============================================================================
// Tabela 8.3 e enforced no frontend via getValidationTables (allowedPatternsSiso).
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
