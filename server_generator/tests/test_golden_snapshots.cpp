// =============================================================================
// Golden snapshots — comparacao "input + output esperado" para os 81 cenarios
// SISO_VALID da Table 8.3.
//
// Estrutura:
//
//   1) [.generate-golden] (oculto por padrao no Catch2)
//      Gerador de fixture. Para cada cenario faz setConfig +
//      computeResultsData, captura o JSON completo, e escreve em
//      tests/fixtures/golden_results_siso.json. Rodar manualmente para
//      atualizar/recapturar:
//
//          ./build/tests/atsc_tests "[.generate-golden]"
//
//   2) "Golden comparison: SISO valid scenarios match captured snapshots"
//      Teste real (executado por padrao). Le o fixture commitado e, para
//      cada cenario, re-roda o pipeline e compara campo-a-campo (deep
//      compare com tolerancia para floats). Qualquer regressao no
//      calculador faz o teste falhar mostrando exatamente qual campo
//      mudou.
//
// Path do fixture e injetado em build-time via GOLDEN_FIXTURES_DIR para que
// o gerador escreva direto na arvore de fontes (commitavel) e o teste leia
// do mesmo lugar, independente do CWD do ctest.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "siso_scenarios.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

#ifndef GOLDEN_FIXTURES_DIR
#error "GOLDEN_FIXTURES_DIR must be defined at compile time (see tests/CMakeLists.txt)"
#endif

using siso::Bench;
using siso::SISO_VALID;
using siso::SisoScenario;
using siso::buildSetConfig;
using siso::buildComputeResults;
using siso::scenarioName;
using siso::json;

using Catch::Matchers::WithinAbs;
using Catch::Matchers::WithinRel;

namespace {

constexpr const char* GOLDEN_FILE = "golden_results_siso.json";

std::filesystem::path goldenFilePath() {
    return std::filesystem::path(GOLDEN_FIXTURES_DIR) / GOLDEN_FILE;
}

// Roda o pipeline completo (setConfig + computeResultsData) num Bench novo
// e devolve o JSON de saida.
json runPipeline(const SisoScenario& s) {
    std::filesystem::create_directories("config/log");
    Bench b;
    auto setRes = b.call(buildSetConfig(s));
    REQUIRE(setRes["status"] == "ok");
    return b.call(buildComputeResults(s));
}

// Deep-compare recursivo. Numeros inteiros: equality exata. Floats: WithinAbs
// (1e-6 absoluto) + WithinRel (1e-9 relativo). Strings/bool/null: equality.
// Objects: mesma chave-set, recurse. Arrays: mesmo tamanho, recurse posicional.
//
// path acumula "$.frameDuration.totalDurationMs" para localizar a falha.
void compareJsonDeep(const json& actual, const json& expected, const std::string& path) {
    INFO("path: " << path);

    if (expected.is_null()) {
        CHECK(actual.is_null());
        return;
    }
    if (expected.is_boolean()) {
        REQUIRE(actual.is_boolean());
        CHECK(actual.get<bool>() == expected.get<bool>());
        return;
    }
    if (expected.is_string()) {
        REQUIRE(actual.is_string());
        CHECK(actual.get<std::string>() == expected.get<std::string>());
        return;
    }
    if (expected.is_number_integer() || expected.is_number_unsigned()) {
        REQUIRE(actual.is_number());
        // Aceita int OU float que represente o mesmo inteiro (ex: 25 vs 25.0)
        if (actual.is_number_integer() || actual.is_number_unsigned()) {
            CHECK(actual.get<int64_t>() == expected.get<int64_t>());
        } else {
            CHECK_THAT(actual.get<double>(), WithinAbs((double)expected.get<int64_t>(), 1e-9));
        }
        return;
    }
    if (expected.is_number_float()) {
        REQUIRE(actual.is_number());
        double a = actual.get<double>();
        double e = expected.get<double>();
        // Tolera ate 1e-6 absoluto OU 1e-9 relativo (cobre numeros gigantes
        // como total_cells e numeros pequenos como CNR ~ 2.x).
        CHECK_THAT(a, WithinAbs(e, 1e-6) || WithinRel(e, 1e-9));
        return;
    }
    if (expected.is_array()) {
        REQUIRE(actual.is_array());
        REQUIRE(actual.size() == expected.size());
        for (size_t i = 0; i < expected.size(); i++) {
            compareJsonDeep(actual[i], expected[i], path + "[" + std::to_string(i) + "]");
        }
        return;
    }
    if (expected.is_object()) {
        REQUIRE(actual.is_object());
        for (auto& [k, v] : expected.items()) {
            REQUIRE(actual.contains(k));
            compareJsonDeep(actual.at(k), v, path + "." + k);
        }
        // Tambem checa se o atual tem chaves a mais (regressao "campo novo").
        for (auto& [k, _v] : actual.items()) {
            CHECK(expected.contains(k));
        }
        return;
    }

    FAIL("Tipo JSON nao suportado em path: " << path);
}

} // namespace


// =============================================================================
// GERADOR — executar manualmente para (re)criar o fixture.
//
//   ./build/tests/atsc_tests "[.generate-golden]"
//
// Tag `[.generate-golden]` faz Catch2 esconder esse cenario do default run.
// =============================================================================
SCENARIO("Captura golden snapshots SISO_VALID a partir do build atual",
         "[.generate-golden]") {
    auto path = goldenFilePath();
    INFO("Escrevendo fixture em: " << path);

    json fixture;
    fixture["metadata"] = {
        {"description", "Snapshots de computeResultsData capturados do build atual"},
        {"source", "test_golden_snapshots.cpp [.generate-golden]"},
        {"scenario_count", SISO_VALID.size()},
        {"input_template", {
            {"bsr_coefficient", 2},
            {"frame_lenght_mode", 1},
            {"frame_lenght", 250},
            {"detail_fec_type", 0},
            {"num_ofdm", 30},
            {"plp.fec_type", 1},
            {"plp.mod_order", 0},
            {"plp.code_rate", 7},
            {"plp.size", 100}
        }}
    };

    json scenarios = json::array();
    for (const auto& s : SISO_VALID) {
        json out = runPipeline(s);
        scenarios.push_back({
            {"name", scenarioName(s)},
            {"input", {{"fft", s.fft}, {"gi", s.gi}, {"sp", s.sp}}},
            {"expected", out}
        });
    }
    fixture["scenarios"] = scenarios;

    std::filesystem::create_directories(path.parent_path());
    std::ofstream f(path);
    REQUIRE(f.is_open());
    f << fixture.dump(2);
    f.close();

    SUCCEED("Captured " << SISO_VALID.size() << " scenarios -> " << path);
}


// =============================================================================
// COMPARACAO — roda por padrao no ctest.
// =============================================================================
SCENARIO("Golden comparison: SISO valid scenarios match captured snapshots") {
    auto path = goldenFilePath();
    if (!std::filesystem::exists(path)) {
        WARN("Fixture ausente: " << path
             << ". Rode `./build/tests/atsc_tests \"[.generate-golden]\"` para criar.");
        SKIP("Sem fixture de golden snapshots — nada a comparar.");
    }

    std::ifstream f(path);
    REQUIRE(f.is_open());
    json fixture = json::parse(f);
    f.close();

    REQUIRE(fixture.contains("scenarios"));
    REQUIRE(fixture["scenarios"].is_array());
    REQUIRE(fixture["scenarios"].size() == SISO_VALID.size());

    for (const auto& entry : fixture["scenarios"]) {
        DYNAMIC_SECTION(entry["name"].get<std::string>()) {
            SisoScenario s{
                entry["input"]["fft"].get<int>(),
                entry["input"]["gi"].get<int>(),
                entry["input"]["sp"].get<int>()
            };

            json actual = runPipeline(s);
            const json& expected = entry["expected"];

            compareJsonDeep(actual, expected, "$");
        }
    }
}
