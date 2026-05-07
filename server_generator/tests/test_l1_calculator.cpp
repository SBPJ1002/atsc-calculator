// =============================================================================
// Unit tests for L1_Calculator.
//
// Foco em metodos numericamente deterministicos:
//   - calc_L1B_time_offset (formula fechada)
//   - estimate_L1D_cells_from_regression (regressao linear/sqrt fixa)
//   - calculate_L1D_size_bytes (clamp em 25)
//   - calculate_frame_duration_ms em modo time-aligned (formula 1-linha)
//   - validate_and_adjust_preamble_num_symbols (clamp negativo)
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "atsc_config.h"
#include "l1_calculator.h"

#include <cstdint>

using Catch::Matchers::WithinAbs;

namespace {

ATSC_Config make_default_config() {
    ATSC_Config cfg("/tmp/test_l1_calc_unused.conf");
    cfg.preamble.L1B_num_subframes = 1;
    cfg.preamble.sub_frames.resize(1);
    cfg.preamble.sub_frames[0].plps.resize(1);
    return cfg;
}

} // namespace

TEST_CASE("L1_Calculator::calc_L1B_time_offset zera quando delta_ns == 0", "[l1_calc]") {
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    CHECK(calc.calc_L1B_time_offset(0, 0, 0) == 0);
    CHECK(calc.calc_L1B_time_offset(5, 0, 0) == 0);
}

TEST_CASE("L1_Calculator::calc_L1B_time_offset com bsr=2, 1ms", "[l1_calc]") {
    // Fs = 384000 * (2 + 16) = 6_912_000 Hz
    // delta_ns = 1000*1000 + 0 = 1_000_000 ns = 1 ms
    // num = 1e6 * 6_912_000 + 5e8 = 6.9120000005e12
    // den = 1e9
    // -> 6912 (uint16)
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    CHECK(calc.calc_L1B_time_offset(2, 1000, 0) == 6912);
}

TEST_CASE("L1_Calculator::calc_L1B_time_offset arredonda half-up", "[l1_calc]") {
    // bsr=0 -> Fs = 384000 * 16 = 6_144_000 Hz
    // usec=1, nsec=0 -> delta_ns = 1000
    // num = 1000 * 6_144_000 + 5e8 = 6_644_000_000
    // -> 6 (samples)
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    CHECK(calc.calc_L1B_time_offset(0, 1, 0) == 6);
}

TEST_CASE("L1_Calculator::estimate_L1D_cells_from_regression: FEC mode 1 = linear", "[l1_calc]") {
    // mode 1: cells = 12.0 * x + 474.0
    // x = max(25, bytes)
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    CHECK(calc.estimate_L1D_cells_from_regression(25,  1) == 774);   // 12*25 + 474
    CHECK(calc.estimate_L1D_cells_from_regression(100, 1) == 1674);  // 12*100 + 474

    // bytes < 25 -> clamp
    CHECK(calc.estimate_L1D_cells_from_regression(0,  1) == 774);
    CHECK(calc.estimate_L1D_cells_from_regression(10, 1) == 774);
}

TEST_CASE("L1_Calculator::estimate_L1D_cells_from_regression: FEC mode 2 = sqrt", "[l1_calc]") {
    // mode 2: cells = 107.6082 * sqrt(x) + 43.9439
    // x=25 -> 107.6082*5 + 43.9439 = 538.041 + 43.9439 = 581.9849 -> round = 582
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    CHECK(calc.estimate_L1D_cells_from_regression(25, 2) == 582);
}

TEST_CASE("L1_Calculator::estimate_L1D_cells_from_regression: FEC mode 0 = linear grande", "[l1_calc]") {
    // mode 0: cells = 48.4968 * x + 1574.5629
    // x=25 -> 48.4968*25 + 1574.5629 = 1212.42 + 1574.5629 = 2786.9829 -> 2787
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    CHECK(calc.estimate_L1D_cells_from_regression(25, 0) == 2787);
}

TEST_CASE("L1_Calculator::calculate_L1D_size_bytes nunca devolve abaixo de 25", "[l1_calc]") {
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    int bytes = calc.calculate_L1D_size_bytes();
    CHECK(bytes >= 25);
}

TEST_CASE("L1_Calculator::calculate_frame_duration_ms time-aligned = length * 5", "[l1_calc]") {
    // Modo 0 (time-aligned): duration_ms = L1B_frame_length * 5
    // (ver l1_calculator.cpp:775-782).
    ATSC_Config cfg = make_default_config();
    cfg.preamble.L1B_frame_length_mode = 0;
    cfg.preamble.L1B_frame_length      = 50;

    L1_Calculator calc(cfg);
    CHECK_THAT(calc.calculate_frame_duration_ms(),
               WithinAbs(250.0, 1e-9));

    cfg.preamble.L1B_frame_length = 100;
    CHECK_THAT(calc.calculate_frame_duration_ms(),
               WithinAbs(500.0, 1e-9));
}

TEST_CASE("L1_Calculator::calculate_frame_duration_ms symbol-aligned: bootstrap so", "[l1_calc]") {
    // Cenario minimo: 1 simbolo de bootstrap, 0 simbolos de preambulo extra,
    // 0 OFDM symbols, 0 additional samples. Deve devolver apenas T_BS.
    //
    // T_BS = (2048 + 0) / (system_bandwidth * 1024000)
    //      = 2048 / 6_144_000 ~ 0.333... ms
    ATSC_Config cfg = make_default_config();
    cfg.bootstrap.system_bandwidth = 6;
    cfg.bootstrap.bootstrap_symbol = 0;        // 1 simbolo de BS
    cfg.bootstrap.bsr_coefficient  = 0;
    cfg.preamble.L1B_frame_length_mode = 1;
    cfg.preamble.L1B_preamble_num_symbols = -1; // -> 0 simbolos de preambulo apos +1
    cfg.preamble.L1B_additional_samples = 0;
    cfg.preamble.sub_frames[0].L1B_first_sub_fft_size = 0;          // 8K
    cfg.preamble.sub_frames[0].L1B_first_sub_guard_interval = 0;    // 0 samples
    cfg.preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols = 0;

    L1_Calculator calc(cfg);
    double duration = calc.calculate_frame_duration_ms();

    CHECK_THAT(duration, WithinAbs(2048.0 / 6144.0, 1e-6));   // ~0.3333 ms
}

TEST_CASE("L1_Calculator::validate_and_adjust_preamble_num_symbols clamp em 0", "[l1_calc]") {
    ATSC_Config cfg = make_default_config();
    cfg.preamble.L1B_preamble_num_symbols = -5;

    L1_Calculator calc(cfg);
    calc.validate_and_adjust_preamble_num_symbols();

    CHECK(cfg.preamble.L1B_preamble_num_symbols == 0);
}

TEST_CASE("L1_Calculator::validate_and_adjust_preamble_num_symbols nao mexe valores validos", "[l1_calc]") {
    ATSC_Config cfg = make_default_config();
    cfg.preamble.L1B_preamble_num_symbols = 1;

    L1_Calculator calc(cfg);
    calc.validate_and_adjust_preamble_num_symbols();

    CHECK(cfg.preamble.L1B_preamble_num_symbols == 1);
}

TEST_CASE("L1_Calculator::calc_L1B_L1_Detail_total_cells delega regressao", "[l1_calc]") {
    // calc_L1B_L1_Detail_total_cells(l1d_bytes, fec_mode, _) ==
    //   estimate_L1D_cells_from_regression(l1d_bytes, fec_mode)
    ATSC_Config cfg = make_default_config();
    L1_Calculator calc(cfg);

    auto a = calc.calc_L1B_L1_Detail_total_cells(50, 1, 0);
    auto b = calc.estimate_L1D_cells_from_regression(50, 1);
    CHECK(a == b);
}
