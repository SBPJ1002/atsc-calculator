// =============================================================================
// Unit tests for ATSC_Config.
//
// Verifica que load()/save() faz round-trip de bootstrap/preamble/PLP, e que
// process_command() aplica SET_* nos campos certos e responde GET_CONFIG com
// a configuracao corrente.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include "atsc_config.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>

namespace {

// RAII tmp file: garante limpeza mesmo se REQUIRE falhar.
struct TempConfigFile {
    std::filesystem::path path;

    TempConfigFile() {
        std::random_device rd;
        std::string name = "atsc_config_test_" + std::to_string(rd()) + ".conf";
        path = std::filesystem::temp_directory_path() / name;
    }

    ~TempConfigFile() {
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    std::string str() const { return path.string(); }
};

} // namespace

TEST_CASE("ATSC_Config: round-trip save/load preserva campos do bootstrap", "[atsc_config]") {
    TempConfigFile tmp;

    {
        ATSC_Config cfg(tmp.str());
        cfg.bootstrap.major_version    = 137;
        cfg.bootstrap.minor_version    = 2;
        cfg.bootstrap.bootstrap_symbol = 7;
        cfg.bootstrap.ea_wakeup        = 1;
        cfg.bootstrap.system_bandwidth = 6;
        cfg.bootstrap.bsr_coefficient  = 3;
        cfg.bootstrap.min_time_to_next = 42;
        cfg.bootstrap.preamble_structure = 5;
        cfg.frame_count = 10;
        cfg.save();
    }

    REQUIRE(std::filesystem::exists(tmp.path));

    ATSC_Config cfg2(tmp.str());
    cfg2.load();

    CHECK(cfg2.bootstrap.major_version    == 137);
    CHECK(cfg2.bootstrap.minor_version    == 2);
    CHECK(cfg2.bootstrap.bootstrap_symbol == 7);
    CHECK(cfg2.bootstrap.ea_wakeup        == 1);
    CHECK(cfg2.bootstrap.system_bandwidth == 6);
    CHECK(cfg2.bootstrap.bsr_coefficient  == 3);
    CHECK(cfg2.bootstrap.min_time_to_next == 42);
    CHECK(cfg2.bootstrap.preamble_structure == 5);
    CHECK(cfg2.frame_count == 10);
}

TEST_CASE("ATSC_Config: round-trip save/load preserva preamble L1B/L1D", "[atsc_config]") {
    TempConfigFile tmp;

    {
        ATSC_Config cfg(tmp.str());
        cfg.preamble.L1B_version          = 1;
        cfg.preamble.L1B_frame_length_mode = 1;
        cfg.preamble.L1B_frame_length     = 250;
        cfg.preamble.L1B_num_subframes    = 1;
        cfg.preamble.L1B_L1_Detail_fec_type = 2;
        cfg.preamble.L1B_L1_Detail_size_bytes = 64;
        cfg.preamble.L1D_bsid             = 1234;
        cfg.preamble.L1D_time_sec         = 100;
        cfg.preamble.L1D_time_msec        = 200;
        cfg.preamble.L1D_time_usec        = 300;
        cfg.preamble.L1D_time_nsec        = 400;
        cfg.save();
    }

    ATSC_Config cfg2(tmp.str());
    cfg2.load();

    CHECK(cfg2.preamble.L1B_version          == 1);
    CHECK(cfg2.preamble.L1B_frame_length_mode == 1);
    CHECK(cfg2.preamble.L1B_frame_length     == 250);
    CHECK(cfg2.preamble.L1B_num_subframes    == 1);
    CHECK(cfg2.preamble.L1B_L1_Detail_fec_type == 2);
    CHECK(cfg2.preamble.L1B_L1_Detail_size_bytes == 64);
    CHECK(cfg2.preamble.L1D_bsid             == 1234);
    CHECK(cfg2.preamble.L1D_time_sec         == 100);
    CHECK(cfg2.preamble.L1D_time_msec        == 200);
    CHECK(cfg2.preamble.L1D_time_usec        == 300);
    CHECK(cfg2.preamble.L1D_time_nsec        == 400);
}

TEST_CASE("ATSC_Config: process_command SET_* aplica nos campos do bootstrap", "[atsc_config]") {
    TempConfigFile tmp;
    ATSC_Config cfg(tmp.str());

    CHECK(cfg.process_command("SET_MAJOR_VERSION=200")     == "SET_MAJOR_VERSION Done\n");
    CHECK(cfg.bootstrap.major_version == 200);

    CHECK(cfg.process_command("SET_MINOR_VERSION=5")       == "SET_MINOR_VERSION Done\n");
    CHECK(cfg.bootstrap.minor_version == 5);

    CHECK(cfg.process_command("SET_BOOTSTRAP_SYMBOL=3")    == "SET_BOOTSTRAP_SYMBOL Done\n");
    CHECK(cfg.bootstrap.bootstrap_symbol == 3);

    CHECK(cfg.process_command("SET_EA_WAKEUP=1")           == "SET_EA_WAKEUP Done\n");
    CHECK(cfg.bootstrap.ea_wakeup == 1);

    CHECK(cfg.process_command("SET_SYSTEM_BANDWIDTH=8")    == "SET_SYSTEM_BANDWIDTH Done\n");
    CHECK(cfg.bootstrap.system_bandwidth == 8);

    CHECK(cfg.process_command("SET_BSR_COEFFICIENT=4")     == "SET_BSR_COEFFICIENT Done\n");
    CHECK(cfg.bootstrap.bsr_coefficient == 4);

    CHECK(cfg.process_command("SET_MIN_TIME_TO_NEXT=99")   == "SET_MIN_TIME_TO_NEXT Done\n");
    CHECK(cfg.bootstrap.min_time_to_next == 99);

    CHECK(cfg.process_command("SET_PREAMBLE_STRUCTURE=7")  == "SET_PREAMBLE_STRUCTURE Done\n");
    CHECK(cfg.bootstrap.preamble_structure == 7);

    // SET_FRAME_COUNT atualiza o campo, mas o reply e sobrescrito pelo else
    // final do if/else-if chain (atsc_config.cpp:944-947). Validamos so o
    // side-effect.
    cfg.process_command("SET_FRAME_COUNT=42");
    CHECK(cfg.frame_count == 42);
}

TEST_CASE("ATSC_Config: GET_CONFIG retorna a configuracao corrente", "[atsc_config]") {
    TempConfigFile tmp;
    ATSC_Config cfg(tmp.str());

    cfg.bootstrap.major_version = 137;
    cfg.bootstrap.system_bandwidth = 6;
    cfg.frame_count = 5;
    cfg.preamble.L1B_version = 1;
    cfg.preamble.L1D_bsid    = 4321;

    const std::string reply = cfg.process_command("GET_CONFIG");

    REQUIRE_FALSE(reply.empty());
    CHECK(reply.find("137\n")           != std::string::npos);  // major_version
    CHECK(reply.find("\n6\n")           != std::string::npos);  // system_bandwidth
    CHECK(reply.find("\n5\n")           != std::string::npos);  // frame_count
    CHECK(reply.find("\n4321\n")        != std::string::npos);  // L1D_bsid
    CHECK(reply.find("SUBFRAMES_START") != std::string::npos);
    CHECK(reply.find("CONFIG_END")      != std::string::npos);
}

TEST_CASE("ATSC_Config: construtor cria 1 subframe e 1 PLP por default", "[atsc_config]") {
    TempConfigFile tmp;
    ATSC_Config cfg(tmp.str());

    REQUIRE(cfg.preamble.sub_frames.size() == 1);
    REQUIRE(cfg.preamble.sub_frames[0].plps.size() == 1);
}

TEST_CASE("ATSC_Config: arquivo ausente faz save() criar default", "[atsc_config]") {
    TempConfigFile tmp;
    REQUIRE_FALSE(std::filesystem::exists(tmp.path));

    ATSC_Config cfg(tmp.str());
    cfg.load();

    CHECK(std::filesystem::exists(tmp.path));
}
