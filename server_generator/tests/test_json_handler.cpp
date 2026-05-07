// =============================================================================
// Unit tests for JsonHandler.
//
// Cobre as flags individuais (calculatePreambleFields, computePlpCapacity,
// getConfig) e os caminhos de erro (JSON malformado, flag desconhecida, flag
// ausente). O teste end-to-end maior fica em test_e2e_atsc.cpp — aqui isolamos
// cada handler para falhar pontualmente quando algo regredir.
// =============================================================================

#include <catch2/catch_test_macros.hpp>

#include "atsc_config.h"
#include "json_handler.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "nlohmann/json.hpp"

#include <filesystem>

using nlohmann::json;

namespace {

struct Bench {
    ATSC_Config    config{"/tmp/test_json_handler_unused.conf"};
    L1_Calculator  calc{config};
    L1_Generator   gen{config, calc};
    L1_Interpreter interp{config};
    JsonHandler    handler{config, calc, gen, interp};

    json call(const json& req) {
        return json::parse(handler.processRequest(req.dump()));
    }
};

} // namespace

TEST_CASE("JsonHandler: JSON malformado vira status=erro", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto raw = b.handler.processRequest("not a json");
    auto res = json::parse(raw);

    CHECK(res["status"]    == "erro");
    CHECK(res.contains("mensagem"));
}

TEST_CASE("JsonHandler: payload sem flag vira status=erro", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({{"foo", "bar"}});

    CHECK(res["status"]    == "erro");
    CHECK(res["mensagem"]  == "campo 'flag' ausente");
}

TEST_CASE("JsonHandler: flag desconhecida vira status=erro", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({{"flag", "this_does_not_exist"}});

    CHECK(res["status"] == "erro");
    CHECK(res["mensagem"].get<std::string>().rfind("flag desconhecida", 0) == 0);
}

TEST_CASE("JsonHandler::calculatePreambleFields: 8K/GI5/FEC1 => valores conhecidos", "[json_handler]") {
    // Geramos os esperados a partir das mesmas formulas em json_handler.cpp:
    //   l1bCells (constante) + regressao linear FEC mode 1.
    //   FEC mode 1, l1dBytes=25 -> l1dCells = 12*25 + 474 = 774.
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({
        {"flag", "calculatePreambleFields"},
        {"fftValue",                 0},   // 8K
        {"giValue",                  5},
        {"preambleReducedCarriers",  0},
        {"preambleStructure",        0},
        {"l1dFecMode",               1},
        {"l1dBytes",                25}
    });

    CHECK(res["status"]    == "ok");
    CHECK(res["fftName"]   == "8K");
    CHECK(res["l1dCells"].get<int>() == 774);
    CHECK(res["l1dBytes"].get<int>() == 25);
    CHECK(res["numSymbols"].get<int>() >= 1);
    CHECK(res["totalPreambleCells"].get<int>() ==
          res["cellsInFirst"].get<int>() +
          (res["numSymbols"].get<int>() - 1) * res["cellsInNext"].get<int>());
}

TEST_CASE("JsonHandler::calculatePreambleFields: l1dBytes < 25 sobe pra 25", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({
        {"flag", "calculatePreambleFields"},
        {"fftValue",   0},
        {"giValue",    5},
        {"l1dFecMode", 1},
        {"l1dBytes",   0}     // < 25
    });

    CHECK(res["status"]              == "ok");
    CHECK(res["l1dBytes"].get<int>() == 25);
    CHECK(res["l1dCells"].get<int>() == 774);
}

TEST_CASE("JsonHandler::calculatePreambleFields: usa defaults se faltarem campos", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({{"flag", "calculatePreambleFields"}});

    // Defaults (json_handler.cpp:131-137): fft=0,gi=5,cred=0,struct=0,fec=0,bytes=25.
    // FEC mode 0 => 48.4968*25 + 1574.5629 = 2786.98 -> 2787.
    CHECK(res["status"] == "ok");
    CHECK(res["l1dCells"].get<int>() == 2787);
    CHECK(res["l1dBytes"].get<int>() == 25);
    CHECK(res["fftName"] == "8K");
}

TEST_CASE("JsonHandler::computePlpCapacity: zero data symbols zera capacidade", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({
        {"flag", "computePlpCapacity"},
        {"fft",         "8K"},
        {"cred",        0},
        {"spLabelText", "0"},
        {"spBoost",     0},
        {"numSymbols",  0},
        {"sbsFirst",    false},
        {"sbsLast",     false}
    });

    CHECK(res["status"]                 == "ok");
    CHECK(res["nDataSymbols"].get<int>() == 0);
    CHECK(res["nSbsSymbols"].get<int>()  == 0);
    CHECK(res["capacity"].get<int>()     == 0);
}

TEST_CASE("JsonHandler::computePlpCapacity: capacity = nDataSymbols * cps com sbs=false", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({
        {"flag", "computePlpCapacity"},
        {"fft",         "8K"},
        {"cred",        0},
        {"spLabelText", "0"},
        {"spBoost",     0},
        {"numSymbols",  10},
        {"sbsFirst",    false},
        {"sbsLast",     false}
    });

    REQUIRE(res["status"] == "ok");

    int cps          = res["cps"].get<int>();
    int nDataSymbols = res["nDataSymbols"].get<int>();
    int nSbsSymbols  = res["nSbsSymbols"].get<int>();
    int capacity     = res["capacity"].get<int>();

    CHECK(nDataSymbols == 10);
    CHECK(nSbsSymbols  == 0);
    CHECK(capacity     == nDataSymbols * cps);
}

TEST_CASE("JsonHandler::getConfig responde objeto JSON", "[json_handler]") {
    std::filesystem::create_directories("config/log");
    Bench b;

    auto res = b.call({{"flag", "getConfig"}});

    REQUIRE(res.is_object());
    // getConfig pode usar 'status' ou nao — o que importa e que parseou.
    // Ja excluimos o caminho de erro porque mensagem != "campo 'flag' ausente".
    if (res.contains("status")) {
        CHECK(res["status"] != "erro");
    }
}
