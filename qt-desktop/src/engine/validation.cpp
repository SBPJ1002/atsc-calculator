#include "engine/validation.h"
#include <map>

namespace Validation {

static bool is64K(int fec_type) { return (fec_type % 2) == 1; }

static const std::map<int, std::map<int, std::vector<int>>> allowedPatternsSiso = {
    {0, {
        {1, {0,1}}, {2, {0,1,2,3}}, {3, {0,1,2,3}}, {4, {0,1,2,3,4,5}},
        {5, {0,1,2,3,4,5,6,7}}, {6, {0,1,2,3,4,5,6,7,8,9}},
        {7, {0,1,2,3,4,5,6,7,8,9,10,11}}, {8, {0,1,2,3,4,5,6,7,8,9,10,11}},
        {9, {0,1,2,3,4,5,6,7,8,9,10,11,12,13}}, {10, {0,1,2,3,4,5,6,7,8,9,10,11,12,13}},
        {11, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}}, {12, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}}
    }},
    {1, {
        {1, {0,1}}, {2, {0,1,2,3}}, {3, {0,1,2,3}}, {4, {0,1,2,3,4,5}},
        {5, {0,1,2,3,4,5,6,7}}, {6, {0,1,2,3,4,5,6,7,8,9}},
        {7, {0,1,2,3,4,5,6,7,8,9,10,11}}, {8, {0,1,2,3,4,5,6,7,8,9,10,11}},
        {9, {0,1,2,3,4,5,6,7,8,9,10,11,12,13}}, {10, {0,1,2,3,4,5,6,7,8,9,10,11,12,13}},
        {11, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}}, {12, {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}}
    }},
    {2, {
        {1, {0,1}}, {2, {0,1}}, {3, {0,1}}, {4, {0,1,4,5}},
        {5, {0,1,4,5,6,7}}, {6, {0,1,4,5,6,7,8,9}},
        {7, {0,1,4,5,6,7,8,9,10,11}}, {8, {0,1,4,5,6,7,8,9,10,11}},
        {9, {0,1,4,5,6,7,8,9,10,11,12,13}}, {10, {0,1,4,5,6,7,8,9,10,11,12,13}},
        {11, {0,1,4,5,6,7,8,9,10,11,12,13,14,15}}, {12, {0,1,4,5,6,7,8,9,10,11,12,13,14,15}}
    }}
};

static const std::map<int, std::vector<int>> modcod_64K = {
    {0, {0,1,2,3,4,5,6,7,9}},
    {1, {2,3,5,6,7,9}},
    {2, {1,2,3,4,5,6,7,8,9}},
    {3, {2,3,5,6,7,8,9,10,11}},
    {4, {3,5,6,7,8,9,10,11}},
    {5, {5,7,9,10,11}}
};
static const std::map<int, std::vector<int>> modcod_16K = {
    {0, {0,1,2,3,4,5,6,7}},
    {1, {3,4,5,6,9}},
    {2, {3,4,5,6,7,8,9}},
    {3, {3,5,6,7,8,9,10,11}}
};

std::vector<int> getAllowedPilotPatterns(int fft_size, int guard_interval) {
    auto fftIt = allowedPatternsSiso.find(fft_size);
    if (fftIt == allowedPatternsSiso.end()) return {};
    auto giIt = fftIt->second.find(guard_interval);
    if (giIt == fftIt->second.end()) return {};
    return giIt->second;
}

std::vector<int> getValidCodeRates(int fec_type, int mod_order) {
    const auto& table = is64K(fec_type) ? modcod_64K : modcod_16K;
    auto it = table.find(mod_order);
    if (it != table.end()) return it->second;
    return {};
}

bool isValidModCod(int fec_type, int mod_order, int code_rate) {
    auto rates = getValidCodeRates(fec_type, mod_order);
    for (int r : rates) if (r == code_rate) return true;
    return false;
}

QString fecTypeName(int v) {
    switch(v) {
        case 0: return "BCH+16K LDPC"; case 1: return "BCH+64K LDPC";
        case 2: return "CRC+16K LDPC"; case 3: return "CRC+64K LDPC";
        case 4: return "16K LDPC only"; case 5: return "64K LDPC only";
        default: return "Reserved";
    }
}

QString modName(int v) {
    switch(v) {
        case 0: return "QPSK"; case 1: return "16QAM"; case 2: return "64QAM";
        case 3: return "256QAM"; case 4: return "1024QAM"; case 5: return "4096QAM";
        default: return "Reserved";
    }
}

QString codeRateName(int v) {
    static const char* names[] = {"2/15","3/15","4/15","5/15","6/15","7/15","8/15","9/15","10/15","11/15","12/15","13/15"};
    if (v >= 0 && v < 12) return names[v];
    return "Reserved";
}

}
