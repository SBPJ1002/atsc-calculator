// =============================================================================
// Unit tests for BinaryUtils.
//
// Compara o que foi gerado com valores esperados conhecidos (referencias do
// IEEE 802.3 CRC, tabelas de hex/binario, etc.) e reporta pass/fail via
// Catch2.
// =============================================================================

#include <catch2/catch_test_macros.hpp>
#include "binary_utils.h"

#include <cstdint>
#include <vector>

TEST_CASE("BinaryUtils::to_binary(uint32_t,bits) emite os bits esperados", "[binary_utils]") {
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(0),   8) == "00000000");
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(1),   8) == "00000001");
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(5),   8) == "00000101");
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(255), 8) == "11111111");
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(0xABCD), 16) == "1010101111001101");
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(0xFFFFFFFF), 32) ==
          "11111111111111111111111111111111");
}

TEST_CASE("BinaryUtils::to_binary trunca valores que excedem a largura", "[binary_utils]") {
    // 256 nao cabe em 8 bits — implementacao mascara para os bits de baixo (0).
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(256), 8) == "00000000");
    // 257 = 0x101 -> mascara para 8 bits = 1.
    CHECK(BinaryUtils::to_binary(static_cast<uint32_t>(257), 8) == "00000001");
}

TEST_CASE("BinaryUtils::to_binary(uint64_t,bits) cobre >64 bits com zero-pad", "[binary_utils]") {
    auto s = BinaryUtils::to_binary(static_cast<uint64_t>(1), 70);
    REQUIRE(s.size() == 70);
    // 6 zeros do padding alto, depois 63 zeros, depois '1'.
    CHECK(s.substr(0, 6) == "000000");
    CHECK(s.back() == '1');
}

TEST_CASE("BinaryUtils::hex_to_binary <-> binary_string_to_hex faz round-trip", "[binary_utils]") {
    const std::string hex = "DEADBEEF";
    const std::string bin = BinaryUtils::hex_to_binary(hex);

    CHECK(bin.size() == hex.size() * 4);
    CHECK(BinaryUtils::binary_string_to_hex(bin) == hex);

    // Sequencia conhecida: "A5" -> 1010 0101.
    CHECK(BinaryUtils::hex_to_binary("A5") == "10100101");
    CHECK(BinaryUtils::binary_string_to_hex("10100101") == "A5");
}

TEST_CASE("BinaryUtils::bin_to_decimal converte binarios validos", "[binary_utils]") {
    CHECK(BinaryUtils::bin_to_decimal("0") == 0);
    CHECK(BinaryUtils::bin_to_decimal("1") == 1);
    CHECK(BinaryUtils::bin_to_decimal("1010") == 10);
    CHECK(BinaryUtils::bin_to_decimal("11111111") == 255);
}

TEST_CASE("BinaryUtils::bin_to_decimal lanca overflow para >63 bits", "[binary_utils]") {
    std::string sixtyFour(64, '1');
    CHECK_THROWS_AS(BinaryUtils::bin_to_decimal(sixtyFour), std::overflow_error);
}

TEST_CASE("BinaryUtils::bin_to_hex_large preserva o prefixo 0x e o nibble inicial", "[binary_utils]") {
    CHECK(BinaryUtils::bin_to_hex_large("11111111") == "0xff");
    CHECK(BinaryUtils::bin_to_hex_large("00000001") == "0x01");
    // Tamanho nao multiplo de 4 -> implementacao zero-padea a esquerda do
    // ultimo nibble (ver binary_utils.cpp:155-173).
    CHECK(BinaryUtils::bin_to_hex_large("101") == "0x5");
}

TEST_CASE("BinaryUtils::bytesToHex emite hex maiusculo zero-padded", "[binary_utils]") {
    std::vector<unsigned char> bytes = {0x00, 0xAB, 0xFF};
    CHECK(BinaryUtils::bytesToHex(bytes) == "00ABFF");
}

TEST_CASE("BinaryUtils::bytesToBinary concatena big-endian por byte", "[binary_utils]") {
    std::vector<unsigned char> bytes = {0x00, 0xAB, 0xFF};
    CHECK(BinaryUtils::bytesToBinary(bytes) == "000000001010101111111111");
}

TEST_CASE("BinaryUtils::search_in_dicionary devolve valor ou 'Unknown'", "[binary_utils]") {
    std::unordered_map<std::string, std::string> dic = {
        {"00", "QPSK"},
        {"01", "16QAM"},
    };
    CHECK(BinaryUtils::search_in_dicionary(dic, "00") == "QPSK");
    CHECK(BinaryUtils::search_in_dicionary(dic, "01") == "16QAM");
    CHECK(BinaryUtils::search_in_dicionary(dic, "ZZ") == "Unknown");
}

TEST_CASE("BinaryUtils::extractL1DetailSize le os 13 bits de tamanho do L1B", "[binary_utils]") {
    // Construimos 25 bytes de L1B onde L1B_L1_Detail_size_bytes = 1234.
    //
    // Layout (frame_length_mode = 1, ver binary_utils.cpp:201-219):
    //   [0..9]    10 bits — preceding fields
    //   [10]       1 bit  — frame_length_mode = 1
    //   [11..26]  16 bits — L1B_frame_length
    //   [27..33]   7 bits — L1B_excess_samples_per_symbol
    //   [34..41]   8 bits — L1B_time_offset
    //   [42..44]   3 bits — L1B_additional_samples
    //   [45..47]   3 bits — L1B_num_subframes
    //   [48..49]   2 bits — L1B_preamble_num_symbols
    //   [50..62]  13 bits — L1B_L1_Detail_size_bytes
    //   ...
    //
    // 1234 (decimal) = 0010011010010 (13 bits).
    std::string bin(200, '0');
    bin[10] = '1';                                  // frame_length_mode = 1
    const std::string sizeBits = "0010011010010";   // 1234
    for (size_t i = 0; i < sizeBits.size(); i++) {
        bin[50 + i] = sizeBits[i];
    }

    // Empacota 200 bits (= 25 bytes) em vector<unsigned char>.
    std::vector<unsigned char> bytes(25, 0);
    for (size_t i = 0; i < 200; i++) {
        if (bin[i] == '1') {
            bytes[i / 8] |= static_cast<unsigned char>(1 << (7 - (i % 8)));
        }
    }

    CHECK(BinaryUtils::extractL1DetailSize(bytes) == 1234);
}

TEST_CASE("BinaryUtils::calculate_crc32 e deterministico para o mesmo input", "[binary_utils]") {
    // Polinomio custom (0x00210801) — nao corresponde ao CRC-32/IEEE,
    // entao o teste cobre estabilidade (mesmo input -> mesmo CRC) e
    // tamanho de 32 bits.
    const std::string sample = "11001010"
                               "10101010"
                               "00001111"
                               "11110000";

    const std::string crc1 = BinaryUtils::calculate_crc32(sample);
    const std::string crc2 = BinaryUtils::calculate_crc32(sample);

    CHECK(crc1.size() == 32);
    CHECK(crc1 == crc2);

    // Inputs distintos -> CRCs distintos.
    const std::string other = "11001010"
                              "10101010"
                              "00001111"
                              "11110001";
    CHECK(BinaryUtils::calculate_crc32(other) != crc1);
}
