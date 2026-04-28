#include "json_handler.h"
#include "atsc_tables.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <regex>
#include <algorithm>
#include <iomanip>
#include <filesystem>

JsonHandler::JsonHandler(ATSC_Config& config, L1_Calculator& calculator,
                         L1_Generator& generator, L1_Interpreter& interpreter)
    : config(config), calculator(calculator), generator(generator), interpreter(interpreter)
{
}

std::string JsonHandler::processRequest(const std::string& jsonStr) {
    json response;

    try {
        json req = json::parse(jsonStr);

        if (!req.contains("flag")) {
            response["status"] = "erro";
            response["mensagem"] = "campo 'flag' ausente";
            return response.dump() + "\n";
        }

        std::string flag = req["flag"].get<std::string>();
        std::cout << "JSON flag: " << flag << std::endl;

        if (flag == "calculatePreambleFields") {
            response = handleCalculatePreambleFields(req);
        } else if (flag == "computePlpCapacity") {
            response = handleComputePlpCapacity(req);
        } else if (flag == "fillPlpSize") {
            response = handleFillPlpSize(req);
        } else if (flag == "batchFillAllPlps") {
            response = handleBatchFillAllPlps(req);
        } else if (flag == "getL1dSizeBytes") {
            response = handleGetL1dSizeBytes(req);
        } else if (flag == "getFrame2LogData") {
            response = handleGetFrame2LogData(req);
        } else if (flag == "getFrameDuration") {
            response = handleGetFrameDuration(req);
        } else if (flag == "getValidationTables") {
            response = handleGetValidationTables(req);
        } else if (flag == "setConfig") {
            response = handleSetConfig(req);
        } else if (flag == "getConfig") {
            response = handleGetConfig(req);
        } else if (flag == "computeResultsData") {
            response = handleComputeResultsData(req);
        } else {
            response["status"] = "erro";
            response["mensagem"] = "flag desconhecida: " + flag;
        }

    } catch (const json::type_error& e) {
        response["status"] = "erro";
        response["mensagem"] = std::string("Type error: ") + e.what();
        std::cerr << "TYPE ERROR: " << e.what() << std::endl;
    } catch (const json::parse_error& e) {
        response["status"] = "erro";
        response["mensagem"] = std::string("JSON invalido: ") + e.what();
    } catch (const std::exception& e) {
        response["status"] = "erro";
        response["mensagem"] = std::string("Erro interno: ") + e.what();
    }

    return response.dump() + "\n";
}

// ============================================================================
// Preamble Fields
// ============================================================================

json JsonHandler::computePreambleFields(int fftValue, int giValue, int preambleReducedCarriers,
                                         int preambleStructure, int l1dFecMode, int l1dBytes) {
    auto fftIt = ATSCTables::FFT_VALUE_TO_NAME.find(fftValue);
    std::string fftName = (fftIt != ATSCTables::FFT_VALUE_TO_NAME.end()) ? fftIt->second : "8K";

    auto giIt = ATSCTables::GI_VALUE_TO_SAMPLES.find(giValue);
    int giSamples = (giIt != ATSCTables::GI_VALUE_TO_SAMPLES.end()) ? giIt->second : 1024;

    int preambleDx = ATSCTables::getPreambleDxFromStructure(preambleStructure);

    int cellsInFirst = ATSCTables::getPreambleCells(fftName, giSamples, 4, preambleDx);
    int cellsInNext = ATSCTables::getPreambleCells(fftName, giSamples, preambleReducedCarriers, preambleDx);

    // L1D cells estimation
    int x = std::max(25, l1dBytes);
    double cellsDouble;
    switch (l1dFecMode) {
        case 0: cellsDouble = 48.4968 * x + 1574.5629; break;
        case 1: cellsDouble = 12.0000 * x + 474.0000; break;
        case 2: cellsDouble = 107.6082 * sqrt((double)x) + 43.9439; break;
        case 3: cellsDouble = 3.8087 * x + 242.6673; break;
        case 4: cellsDouble = 2.3328 * x + 145.3357; break;
        case 5: cellsDouble = 1.6913 * x + 81.2802; break;
        case 6: cellsDouble = 1.1896 * x + 54.9410; break;
        default: cellsDouble = 1.1896 * x + 54.9410; break;
    }
    int l1dCells = (int)round(cellsDouble);

    int l1Total = ATSCTables::L1B_CELLS_FIXED + l1dCells;
    int numPreambleSymbols = (l1Total > cellsInFirst) ? 2 : 1;
    int totalPreambleCells = cellsInFirst + (numPreambleSymbols - 1) * cellsInNext;
    int plpCells = std::max(0, totalPreambleCells - ATSCTables::L1B_CELLS_FIXED - l1dCells);

    json result;
    result["status"] = "ok";
    result["l1bCells"] = ATSCTables::L1B_CELLS_FIXED;
    result["l1dCells"] = l1dCells;
    result["l1dBytes"] = x;
    result["cellsInFirst"] = cellsInFirst;
    result["cellsInNext"] = cellsInNext;
    result["plpCells"] = plpCells;
    result["numSymbols"] = numPreambleSymbols;
    result["totalPreambleCells"] = totalPreambleCells;
    result["fftName"] = fftName;
    result["giSamples"] = giSamples;
    result["preambleReducedCarriers"] = preambleReducedCarriers;
    result["preambleDx"] = preambleDx;

    return result;
}

json JsonHandler::handleCalculatePreambleFields(const json& req) {
    int fftValue = req.value("fftValue", 0);
    int giValue = req.value("giValue", 5);
    int preambleReducedCarriers = req.value("preambleReducedCarriers", 0);
    int preambleStructure = req.value("preambleStructure", 0);
    int l1dFecMode = req.value("l1dFecMode", 0);
    int l1dBytes = req.value("l1dBytes", 25);

    return computePreambleFields(fftValue, giValue, preambleReducedCarriers,
                                  preambleStructure, l1dFecMode, l1dBytes);
}

// ============================================================================
// PLP Capacity
// ============================================================================

json JsonHandler::computePlpCapacity(const std::string& fft, int cred, const std::string& spLabelText,
                                      int spBoost, int numSymbols, bool sbsFirst, bool sbsLast) {
    ATSCTables::SpInfo sp = ATSCTables::parseSpLabel(spLabelText);

    int cps = ATSCTables::getTableCellsPerSymbol(fft, cred, sp.key);
    if (cps == 0) {
        json result;
        result["status"] = "ok";
        result["capacity"] = 0;
        result["reason"] = "Combination not found: " + sp.key + "/" + fft + "/" + std::to_string(cred);
        return result;
    }

    int nDataSymbols = numSymbols - (sbsFirst ? 1 : 0) - (sbsLast ? 1 : 0);
    int nSbsSymbols = (sbsFirst ? 1 : 0) + (sbsLast ? 1 : 0);
    int sbsDataCells = ATSCTables::getSbsActiveDataCells(cred, sp.key, fft, spBoost);

    int capacity = (nDataSymbols * cps) + (nSbsSymbols * sbsDataCells);
    int capacitySBS = nSbsSymbols * sbsDataCells;

    json result;
    result["status"] = "ok";
    result["capacity"] = capacity;
    result["capacitySBS"] = capacitySBS;
    result["cps"] = cps;
    result["nDataSymbols"] = nDataSymbols;
    result["nSbsSymbols"] = nSbsSymbols;
    result["sbsDataCells"] = sbsDataCells;
    result["fft"] = fft;
    result["spBoost"] = spBoost;
    result["cred"] = cred;
    result["reason"] = nullptr;

    return result;
}

json JsonHandler::handleComputePlpCapacity(const json& req) {
    std::string fft = req.value("fft", "8K");
    int cred = req.value("cred", 0);
    std::string spLabelText = req.value("spLabelText", "8");
    int spBoost = req.value("spBoost", 0);
    int numSymbols = req.value("numSymbols", 0);
    bool sbsFirst = req.value("sbsFirst", false);
    bool sbsLast = req.value("sbsLast", false);

    return computePlpCapacity(fft, cred, spLabelText, spBoost, numSymbols, sbsFirst, sbsLast);
}

// ============================================================================
// Fill PLP Size
// ============================================================================

json JsonHandler::handleFillPlpSize(const json& req) {
    json preambleConfig = req.value("preambleConfig", json::object());
    json subframeConfig = req.value("subframeConfig", json::object());

    json preambleFields = computePreambleFields(
        preambleConfig.value("fftValue", 0),
        preambleConfig.value("giValue", 5),
        preambleConfig.value("preambleReducedCarriers", 0),
        preambleConfig.value("preambleStructure", 0),
        preambleConfig.value("l1dFecMode", 0),
        preambleConfig.value("l1dBytes", 25)
    );

    int plpCells = preambleFields.value("plpCells", 0);

    json plpCapacity = computePlpCapacity(
        subframeConfig.value("fft", "8K"),
        subframeConfig.value("cred", 0),
        subframeConfig.value("spLabelText", "8"),
        subframeConfig.value("spBoost", 0),
        subframeConfig.value("numSymbols", 0),
        subframeConfig.value("sbsFirst", false),
        subframeConfig.value("sbsLast", false)
    );

    int capacity = plpCapacity.value("capacity", 0);
    if (capacity <= 0) {
        json result;
        result["status"] = "ok";
        result["error"] = true;
        result["reason"] = plpCapacity.value("reason", "Unknown error");
        result["totalSize"] = 0;
        result["plpCapacity"] = plpCapacity;
        result["plpCells"] = plpCells;
        result["preambleFields"] = preambleFields;
        return result;
    }

    int totalSize = capacity + plpCells;

    json result;
    result["status"] = "ok";
    result["error"] = false;
    result["totalSize"] = totalSize;
    result["plpCapacity"] = plpCapacity;
    result["plpCells"] = plpCells;
    result["preambleFields"] = preambleFields;

    return result;
}

// ============================================================================
// Batch Fill All PLPs
// ============================================================================

json JsonHandler::handleBatchFillAllPlps(const json& req) {
    json preambleConfig = req.value("preambleConfig", json::object());
    json subframesConfigs = req.value("subframesConfigs", json::array());

    json preambleFields = computePreambleFields(
        preambleConfig.value("fftValue", 0),
        preambleConfig.value("giValue", 5),
        preambleConfig.value("preambleReducedCarriers", 0),
        preambleConfig.value("preambleStructure", 0),
        preambleConfig.value("l1dFecMode", 0),
        preambleConfig.value("l1dBytes", 25)
    );

    int plpCells = preambleFields.value("plpCells", 0);
    json results = json::array();

    for (const auto& sfConfig : subframesConfigs) {
        json plps = sfConfig.value("plps", json::array());
        if (plps.empty()) {
            plps = json::array({sfConfig});
        }

        json sfResults = json::array();
        for (const auto& plpConfig : plps) {
            json plpCapacity = computePlpCapacity(
                plpConfig.value("fft", "8K"),
                plpConfig.value("cred", 0),
                plpConfig.value("spLabelText", "8"),
                plpConfig.value("spBoost", 0),
                plpConfig.value("numSymbols", 0),
                plpConfig.value("sbsFirst", false),
                plpConfig.value("sbsLast", false)
            );

            int capacity = plpCapacity.value("capacity", 0);
            if (capacity <= 0) {
                sfResults.push_back({
                    {"error", true},
                    {"reason", plpCapacity.value("reason", "")},
                    {"totalSize", 0},
                    {"plpCapacity", plpCapacity}
                });
            } else {
                sfResults.push_back({
                    {"error", false},
                    {"totalSize", capacity + plpCells},
                    {"plpCapacity", plpCapacity}
                });
            }
        }
        results.push_back(sfResults);
    }

    json response;
    response["status"] = "ok";
    response["results"] = results;
    response["preambleFields"] = preambleFields;
    response["plpCells"] = plpCells;

    return response;
}

// ============================================================================
// Get L1D Size Bytes (from log files)
// ============================================================================

json JsonHandler::handleGetL1dSizeBytes(const json& req) {
    json response;
    response["status"] = "ok";

    std::string logDir = "config/log/";

    // Find most recent Frame_*.log file
    std::string bestFile;
    std::filesystem::file_time_type bestTime{};

    try {
        for (const auto& entry : std::filesystem::directory_iterator(logDir)) {
            std::string fname = entry.path().filename().string();
            if (fname.find("Frame_") == 0 && fname.find(".log") != std::string::npos) {
                auto ftime = entry.last_write_time();
                if (bestFile.empty() || ftime > bestTime) {
                    bestFile = entry.path().string();
                    bestTime = ftime;
                }
            }
        }
    } catch (...) {
        response["error"] = "Log directory not found";
        response["bytes"] = 25;
        return response;
    }

    if (bestFile.empty()) {
        response["error"] = "No log files found";
        response["bytes"] = 25;
        return response;
    }

    std::ifstream file(bestFile);
    if (!file.is_open()) {
        response["error"] = "Cannot read log file";
        response["bytes"] = 25;
        return response;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    std::regex pattern(R"(L1B_L1_Detail_size_bytes\s*→\s*(\d+)\s*bytes)");
    std::smatch match;
    if (std::regex_search(content, match, pattern)) {
        int bytes = std::max(25, std::stoi(match[1].str()));
        response["bytes"] = bytes;
        response["source"] = std::filesystem::path(bestFile).filename().string();
    } else {
        response["error"] = "L1B_L1_Detail_size_bytes not found in log";
        response["bytes"] = 25;
    }

    return response;
}

// ============================================================================
// Get Frame 2 Log Data
// ============================================================================

json JsonHandler::handleGetFrame2LogData(const json& req) {
    json response;
    response["status"] = "ok";

    std::string logFile = "config/log/Frame_2.log";

    std::ifstream file(logFile);
    if (!file.is_open()) {
        response["error"] = "Frame_2.log not found";
        return response;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();

    // Parse time offset
    std::regex toPattern(R"(L1B_time_offset\s*→\s*(\d+))");
    std::smatch toMatch;
    if (std::regex_search(content, toMatch, toPattern)) {
        response["timeOffset"] = std::stoi(toMatch[1].str());
    } else {
        response["timeOffset"] = nullptr;
    }

    // Parse FEC block starts
    json fecBlockStarts = json::array();
    std::regex plpPattern(R"(L1D_plp_id\s*→\s*(\d+)([\s\S]*?)(?=L1D_plp_id|L1D_reserved|$))");
    std::regex fecPattern(R"(L1D_plp_fec_block_start\s*→\s*(\d+))");

    auto begin = std::sregex_iterator(content.begin(), content.end(), plpPattern);
    auto end = std::sregex_iterator();

    for (auto it = begin; it != end; ++it) {
        int plpId = std::stoi((*it)[1].str());
        std::string plpSection = (*it)[2].str();

        json fecEntry;
        fecEntry["plpId"] = plpId;

        std::smatch fecMatch;
        if (std::regex_search(plpSection, fecMatch, fecPattern)) {
            fecEntry["fecBlockStart"] = std::stoi(fecMatch[1].str());
        } else {
            fecEntry["fecBlockStart"] = nullptr;
        }

        fecBlockStarts.push_back(fecEntry);
    }

    response["fecBlockStarts"] = fecBlockStarts;
    response["source"] = "Frame_2.log";

    return response;
}

// ============================================================================
// Get Frame Duration
// ============================================================================

json JsonHandler::handleGetFrameDuration(const json& req) {
    json response;
    response["status"] = "ok";

    std::string durationFile = "config/log/frame_duration.txt";

    std::ifstream file(durationFile);
    if (!file.is_open()) {
        response["error"] = "frame_duration.txt not found";
        response["totalDurationMs"] = nullptr;
        response["subframeDurations"] = json::object();
        return response;
    }

    std::string line;
    json subframeDurations = json::object();

    while (std::getline(file, line)) {
        // trim
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        if (line.empty()) continue;

        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string val = line.substr(eqPos + 1);

        if (key == "frame_duration_ms") {
            response["totalDurationMs"] = std::stod(val);
        } else if (key.find("subframe_") == 0 && key.find("_duration_ms") != std::string::npos) {
            // Extract index: subframe_X_duration_ms
            size_t start = 9; // length of "subframe_"
            size_t end = key.find("_duration_ms");
            std::string idxStr = key.substr(start, end - start);
            subframeDurations[idxStr] = std::stod(val);
        }
    }

    file.close();
    response["subframeDurations"] = subframeDurations;

    if (!response.contains("totalDurationMs")) {
        response["totalDurationMs"] = nullptr;
    }

    return response;
}

// ============================================================================
// Get Validation Tables
// ============================================================================

json JsonHandler::handleGetValidationTables(const json& req) {
    json response;
    response["status"] = "ok";

    // Pilot pattern maps
    json ppValueToName = json::object();
    json ppNameToValue = json::object();
    for (const auto& [k, v] : ATSCTables::SP_PATTERN_MAP) {
        ppValueToName[k] = v;
        ppNameToValue[v] = k;
    }
    response["pilotPatternValueToName"] = ppValueToName;
    response["pilotPatternNameToValue"] = ppNameToValue;

    // All pilot patterns
    response["allPilotPatterns"] = json::array({"0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15"});

    // Allowed patterns SISO
    json allowedSiso;
    allowedSiso["8K"] = {
        {"GI1_192", json::array({"10","11","14","15"})},
        {"GI2_384", json::array({"6","7","10","11"})},
        {"GI3_512", json::array({"4","5","8","9"})},
        {"GI4_768", json::array({"2","3","6","7"})},
        {"GI5_1024", json::array({"0","1","4","5"})},
        {"GI6_1536", json::array({"2","3"})},
        {"GI7_2048", json::array({"0","1"})},
        {"GI8_2432", json::array()}, {"GI9_3072", json::array()},
        {"GI10_3648", json::array()}, {"GI11_4096", json::array()}, {"GI12_4864", json::array()}
    };
    allowedSiso["16K"] = {
        {"GI1_192", json::array({"14","15"})},
        {"GI2_384", json::array({"10","11","14","15"})},
        {"GI3_512", json::array({"8","9","12","13"})},
        {"GI4_768", json::array({"6","7","10","11"})},
        {"GI5_1024", json::array({"4","5","8","9"})},
        {"GI6_1536", json::array({"2","3","6","7"})},
        {"GI7_2048", json::array({"0","1","4","5"})},
        {"GI8_2432", json::array({"0","1","4","5"})},
        {"GI9_3072", json::array({"2","3"})},
        {"GI10_3648", json::array({"2","3"})},
        {"GI11_4096", json::array({"0","1"})},
        {"GI12_4864", json::array()}
    };
    allowedSiso["32K"] = {
        {"GI1_192", json::array({"14"})}, {"GI2_384", json::array({"14"})},
        {"GI3_512", json::array({"12"})},
        {"GI4_768", json::array({"10","14"})},
        {"GI5_1024", json::array({"8","12"})},
        {"GI6_1536", json::array({"6","10"})},
        {"GI7_2048", json::array({"4","8"})},
        {"GI8_2432", json::array({"4","8"})},
        {"GI9_3072", json::array({"0","6"})},
        {"GI10_3648", json::array({"0","6"})},
        {"GI11_4096", json::array({"0","4"})},
        {"GI12_4864", json::array({"0","4"})}
    };
    response["allowedPatternsSiso"] = allowedSiso;

    // Modulation/Coding tables
    json modCoding;
    modCoding["64K"] = {
        {"0", json::array({"0","1","2","3","4","5","6","7","9"})},
        {"1", json::array({"2","3","5","6","7","9"})},
        {"2", json::array({"1","2","3","4","5","6","7","8","9"})},
        {"3", json::array({"2","3","5","6","7","8","9","10","11"})},
        {"4", json::array({"3","5","6","7","8","9","10","11"})},
        {"5", json::array({"5","7","9","10","11"})}
    };
    modCoding["16K"] = {
        {"0", json::array({"0","1","2","3","4","5","6","7"})},
        {"1", json::array({"3","4","5","6","9"})},
        {"2", json::array({"3","4","5","6","7","8","9"})},
        {"3", json::array({"3","5","6","7","8","9","10","11"})}
    };
    response["modulationCodingTables"] = modCoding;

    response["fecTypeMap"] = {
        {"0","16K"},{"1","64K"},{"2","16K"},{"3","64K"},{"4","16K"},{"5","64K"}
    };
    response["modulationMap"] = {
        {"0","QPSK"},{"1","16QAM"},{"2","64QAM"},{"3","256QAM"},{"4","1024QAM"},{"5","4096QAM"}
    };
    response["codeRateMap"] = {
        {"0","2/15"},{"1","3/15"},{"2","4/15"},{"3","5/15"},
        {"4","6/15"},{"5","7/15"},{"6","8/15"},{"7","9/15"},
        {"8","10/15"},{"9","11/15"},{"10","12/15"},{"11","13/15"}
    };

    // Preamble configurations
    struct PreambleEntry { std::string fft; std::string gi; int dx; };
    std::vector<PreambleEntry> entries = {
        {"8K","GI1_192",16},{"8K","GI2_384",8},{"8K","GI3_512",6},
        {"8K","GI4_768",4},{"8K","GI5_1024",3},{"8K","GI6_1536",4},
        {"8K","GI7_2048",3},
        {"16K","GI1_192",32},{"16K","GI2_384",16},{"16K","GI3_512",12},
        {"16K","GI4_768",8},{"16K","GI5_1024",6},{"16K","GI6_1536",4},
        {"16K","GI7_2048",3},{"16K","GI8_2432",3},{"16K","GI9_3072",4},
        {"16K","GI10_3648",4},{"16K","GI11_4096",3},
        {"32K","GI1_192",32},{"32K","GI2_384",32},{"32K","GI3_512",24},
        {"32K","GI4_768",16},{"32K","GI5_1024",12},{"32K","GI6_1536",8},
        {"32K","GI7_2048",6},{"32K","GI8_2432",6},
        {"32K","GI9_3072",8},{"32K","GI9_3072",3},
        {"32K","GI10_3648",8},{"32K","GI10_3648",3},
        {"32K","GI11_4096",3},{"32K","GI12_4864",3},
    };

    json preambleConfigs = json::array();
    for (const auto& e : entries) {
        for (int mode = 1; mode <= 5; mode++) {
            preambleConfigs.push_back({
                {"fft", e.fft}, {"gi", e.gi}, {"dx", e.dx}, {"mode", mode}
            });
        }
    }
    response["preambleConfigurations"] = preambleConfigs;

    return response;
}

// ============================================================================
// Set Config - applies configuration and regenerates
// ============================================================================

json JsonHandler::handleSetConfig(const json& req) {
    applyConfigFromJson(req);
    regenerateAfterConfig();

    json response;
    response["status"] = "ok";
    response["mensagem"] = "Configuracao aplicada com sucesso";
    return response;
}

void JsonHandler::applyConfigFromJson(const json& req) {
    static const std::map<int,int> MAJOR_VERSION_MAP = {{0,137},{1,400}};
    static const std::map<int,int> BOOTSTRAP_SYMBOL_MAP = {{0,4},{1,5}};
    static const std::map<int,int> SYSTEM_BANDWIDTH_MAP = {{0,6},{1,7},{2,8}};

    auto mapValue = [](const std::map<int,int>& m, int key) -> int {
        auto it = m.find(key);
        return (it != m.end()) ? it->second : key;
    };

    if (req.contains("major_version"))
        config.bootstrap.major_version = mapValue(MAJOR_VERSION_MAP, req["major_version"].get<int>());
    if (req.contains("minor_version"))
        config.bootstrap.minor_version = req["minor_version"].get<int>();
    if (req.contains("bootstrap_symbol"))
        config.bootstrap.bootstrap_symbol = mapValue(BOOTSTRAP_SYMBOL_MAP, req["bootstrap_symbol"].get<int>());
    if (req.contains("ea_wakeup"))
        config.bootstrap.ea_wakeup = req["ea_wakeup"].get<int>();
    if (req.contains("system_bandwidth"))
        config.bootstrap.system_bandwidth = mapValue(SYSTEM_BANDWIDTH_MAP, req["system_bandwidth"].get<int>());
    if (req.contains("bsr_coefficient"))
        config.bootstrap.bsr_coefficient = req["bsr_coefficient"].get<int>();
    if (req.contains("min_time_to_next"))
        config.bootstrap.min_time_to_next = req["min_time_to_next"].get<int>();
    if (req.contains("preamble_structure"))
        config.bootstrap.preamble_structure = req["preamble_structure"].get<int>();
    if (req.contains("number_of_frames"))
        config.frame_count = std::max(1, std::min(100, req["number_of_frames"].get<int>()));
    if (req.contains("l1b_version"))
        config.preamble.L1B_version = req["l1b_version"].get<int>();
    if (req.contains("l1b_mimo_scatterred_pilot_encoding"))
        config.preamble.L1B_mimo_scattered_pilot_encoding = req["l1b_mimo_scatterred_pilot_encoding"].get<int>();
    if (req.contains("l1d_version"))
        config.preamble.L1D_version = req["l1d_version"].get<int>();
    if (req.contains("l1d_bsid"))
        config.preamble.L1D_bsid = req["l1d_bsid"].get<int>();
    if (req.contains("detail_fec_type"))
        config.preamble.L1B_L1_Detail_fec_type = req["detail_fec_type"].get<int>();
    if (req.contains("time_info_flag"))
        config.preamble.L1B_time_info_flag = req["time_info_flag"].get<int>();
    if (req.contains("frame_lenght_mode"))
        config.preamble.L1B_frame_length_mode = req["frame_lenght_mode"].get<int>();
    if (req.contains("frame_lenght"))
        config.preamble.L1B_frame_length = req["frame_lenght"].get<int>();
    if (req.contains("number_of_subframes")) {
        int numSf = req["number_of_subframes"].get<int>();
        config.preamble.L1B_num_subframes = numSf;
        config.preamble.sub_frames.resize(numSf);
    }

    // Subframes
    if (req.contains("subframes")) {
        const auto& subframes = req["subframes"];
        for (size_t i = 0; i < subframes.size() && i < config.preamble.sub_frames.size(); i++) {
            const auto& sf = subframes[i];
            auto& target = config.preamble.sub_frames[i];

            if (i == 0) {
                if (sf.contains("plp_mimo")) target.L1B_first_sub_mimo = sf["plp_mimo"].get<int>();
                if (sf.contains("plp_mimo_mixed")) target.L1B_first_sub_mimo_mixed = sf["plp_mimo_mixed"].get<int>();
                if (sf.contains("plp_miso")) target.L1B_first_sub_miso = sf["plp_miso"].get<int>();
                if (sf.contains("fft_size")) target.L1B_first_sub_fft_size = sf["fft_size"].get<int>();
                if (sf.contains("reduced_carrier")) target.L1B_first_sub_reduced_carriers = sf["reduced_carrier"].get<int>();
                if (sf.contains("guard_interval")) target.L1B_first_sub_guard_interval = sf["guard_interval"].get<int>();
                if (sf.contains("num_ofdm")) target.L1B_first_sub_num_ofdm_symbols = sf["num_ofdm"].get<int>();
                if (sf.contains("spilot_pattern")) target.L1B_first_sub_scattered_pilot_pattern = sf["spilot_pattern"].get<int>();
                if (sf.contains("spilot_boost")) target.L1B_first_sub_scattered_pilot_boost = sf["spilot_boost"].get<int>();
                if (sf.contains("sbs_first")) target.L1B_first_sub_sbs_first = sf["sbs_first"].get<int>();
                if (sf.contains("sbs_last")) target.L1B_first_sub_sbs_last = sf["sbs_last"].get<int>();
            } else {
                if (sf.contains("plp_mimo")) target.L1D_mimo = sf["plp_mimo"].get<int>();
                if (sf.contains("plp_mimo_mixed")) target.L1D_mimo_mixed = sf["plp_mimo_mixed"].get<int>();
                if (sf.contains("plp_miso")) target.L1D_miso = sf["plp_miso"].get<int>();
                if (sf.contains("fft_size")) target.L1D_fft_size = sf["fft_size"].get<int>();
                if (sf.contains("reduced_carrier")) target.L1D_reduced_carriers = sf["reduced_carrier"].get<int>();
                if (sf.contains("guard_interval")) target.L1D_guard_interval = sf["guard_interval"].get<int>();
                if (sf.contains("num_ofdm")) target.L1D_num_ofdm_symbols = sf["num_ofdm"].get<int>();
                if (sf.contains("spilot_pattern")) target.L1D_scattered_pilot_pattern = sf["spilot_pattern"].get<int>();
                if (sf.contains("spilot_boost")) target.L1D_scattered_pilot_boost = sf["spilot_boost"].get<int>();
                if (sf.contains("sbs_first")) target.L1D_sbs_first = sf["sbs_first"].get<int>();
                if (sf.contains("sbs_last")) target.L1D_sbs_last = sf["sbs_last"].get<int>();
            }

            if (sf.contains("freq_interleaver")) target.L1D_frequency_interleaver = sf["freq_interleaver"].get<int>();

            // PLPs
            if (sf.contains("plps")) {
                const auto& plps = sf["plps"];
                target.plps.resize(plps.size());
                for (size_t j = 0; j < plps.size(); j++) {
                    const auto& p = plps[j];
                    auto& plp = target.plps[j];

                    if (p.contains("plp_id")) plp.L1D_plp_id = p["plp_id"].get<int>();
                    else if (p.contains("id")) plp.L1D_plp_id = p["id"].get<int>();
                    if (p.contains("lls_flag")) plp.L1D_plp_lls_flag = p["lls_flag"].get<int>();
                    if (p.contains("layer")) plp.L1D_plp_layer = p["layer"].get<int>();
                    if (p.contains("start")) plp.L1D_plp_start = p["start"].get<int>();
                    if (p.contains("size")) plp.L1D_plp_size = p["size"].get<int>();
                    if (p.contains("fec_type")) plp.L1D_plp_fec_type = p["fec_type"].get<int>();
                    if (p.contains("mod_order")) plp.L1D_plp_mod = p["mod_order"].get<int>();
                    if (p.contains("code_rate")) plp.L1D_plp_cod = p["code_rate"].get<int>();
                    if (p.contains("ti_mode")) plp.L1D_plp_TI_mode = p["ti_mode"].get<int>();
                    if (p.contains("ti_extended")) plp.L1D_plp_TI_extended_interleaving = p["ti_extended"].get<int>();
                    if (p.contains("cti_depth")) plp.L1D_plp_CTI_depth = p["cti_depth"].get<int>();
                    if (p.contains("mimo_plp")) plp.L1D_plp_mimo = p["mimo_plp"].get<int>();
                    if (p.contains("stream_combining")) plp.L1D_plp_mimo_stream_combining = p["stream_combining"].get<int>();
                    if (p.contains("iq_intervaling")) plp.L1D_plp_mimo_IQ_interleaving = p["iq_intervaling"].get<int>();
                    if (p.contains("phase_hopping")) plp.L1D_plp_mimo_PH = p["phase_hopping"].get<int>();
                    if (p.contains("plp_type")) plp.L1D_plp_type = p["plp_type"].get<int>();
                    else if (p.contains("type")) plp.L1D_plp_type = p["type"].get<int>();
                    if (p.contains("num_subslice")) plp.L1D_plp_num_subslices = p["num_subslice"].get<int>();
                    if (p.contains("subslice_interval")) plp.L1D_plp_subslice_interval = p["subslice_interval"].get<int>();
                    if (p.contains("cell_intervaler")) plp.L1D_plp_HTI_cell_interleaver = p["cell_intervaler"].get<int>();
                    if (p.contains("inter_subframe")) plp.L1D_plp_HTI_inter_subframe = p["inter_subframe"].get<int>();
                    if (p.contains("num_ti_blocks")) plp.L1D_plp_HTI_num_ti_blocks = p["num_ti_blocks"].get<int>();
                    if (p.contains("num_fec_blocks_max")) plp.L1D_plp_HTI_num_fec_blocks_max = p["num_fec_blocks_max"].get<int>();
                    if (p.contains("num_fec_blocks")) plp.L1D_plp_HTI_num_fec_blocks = p["num_fec_blocks"].get<int>();
                    if (p.contains("ldm_injection_level")) plp.L1D_plp_ldm_injection_level = p["ldm_injection_level"].get<int>();
                }
            }
        }
    }
}

void JsonHandler::regenerateAfterConfig() {
    config.save();
    generator.generate_basic_multi_frame();
    generator.generate_detail_multi_frame();
    interpreter.run();
    calculator.write_frame_duration_file(calculator.calculate_frame_duration_ms());
}

// ============================================================================
// Get Config
// ============================================================================

json JsonHandler::handleGetConfig(const json& req) {
    static const std::map<int,int> MAJOR_VERSION_REVERSE = {{137,0},{400,1}};
    static const std::map<int,int> BOOTSTRAP_SYMBOL_REVERSE = {{4,0},{5,1}};
    static const std::map<int,int> SYSTEM_BANDWIDTH_REVERSE = {{6,0},{7,1},{8,2}};

    auto reverseMap = [](const std::map<int,int>& m, int val) -> int {
        auto it = m.find(val);
        return (it != m.end()) ? it->second : val;
    };

    json response;
    response["status"] = "ok";

    json cfg;
    cfg["major_version"] = reverseMap(MAJOR_VERSION_REVERSE, config.bootstrap.major_version);
    cfg["minor_version"] = config.bootstrap.minor_version;
    cfg["bootstrap_symbol"] = reverseMap(BOOTSTRAP_SYMBOL_REVERSE, config.bootstrap.bootstrap_symbol);
    cfg["ea_wakeup"] = config.bootstrap.ea_wakeup;
    cfg["system_bandwidth"] = reverseMap(SYSTEM_BANDWIDTH_REVERSE, config.bootstrap.system_bandwidth);
    cfg["bsr_coefficient"] = config.bootstrap.bsr_coefficient;
    cfg["min_time_to_next"] = config.bootstrap.min_time_to_next;
    cfg["preamble_structure"] = config.bootstrap.preamble_structure;
    cfg["number_of_frames"] = config.frame_count;
    cfg["l1b_version"] = config.preamble.L1B_version;
    cfg["l1b_mimo_scatterred_pilot_encoding"] = config.preamble.L1B_mimo_scattered_pilot_encoding;
    cfg["detail_fec_type"] = config.preamble.L1B_L1_Detail_fec_type;
    cfg["time_info_flag"] = config.preamble.L1B_time_info_flag;
    cfg["frame_lenght_mode"] = config.preamble.L1B_frame_length_mode;
    cfg["frame_lenght"] = config.preamble.L1B_frame_length;
    cfg["number_of_subframes"] = config.preamble.L1B_num_subframes;
    cfg["l1d_version"] = config.preamble.L1D_version;
    cfg["l1d_bsid"] = config.preamble.L1D_bsid;

    json subframes = json::array();
    for (size_t i = 0; i < config.preamble.sub_frames.size(); i++) {
        json sf;
        const auto& s = config.preamble.sub_frames[i];

        if (i == 0) {
            sf["plp_mimo"] = s.L1B_first_sub_mimo;
            sf["plp_mimo_mixed"] = s.L1B_first_sub_mimo_mixed;
            sf["plp_miso"] = s.L1B_first_sub_miso;
            sf["fft_size"] = s.L1B_first_sub_fft_size;
            sf["reduced_carrier"] = s.L1B_first_sub_reduced_carriers;
            sf["guard_interval"] = s.L1B_first_sub_guard_interval;
            sf["num_ofdm"] = s.L1B_first_sub_num_ofdm_symbols;
            sf["spilot_pattern"] = s.L1B_first_sub_scattered_pilot_pattern;
            sf["spilot_boost"] = s.L1B_first_sub_scattered_pilot_boost;
            sf["sbs_first"] = s.L1B_first_sub_sbs_first;
            sf["sbs_last"] = s.L1B_first_sub_sbs_last;
        } else {
            sf["plp_mimo"] = s.L1D_mimo;
            sf["plp_mimo_mixed"] = s.L1D_mimo_mixed;
            sf["plp_miso"] = s.L1D_miso;
            sf["fft_size"] = s.L1D_fft_size;
            sf["reduced_carrier"] = s.L1D_reduced_carriers;
            sf["guard_interval"] = s.L1D_guard_interval;
            sf["num_ofdm"] = s.L1D_num_ofdm_symbols;
            sf["spilot_pattern"] = s.L1D_scattered_pilot_pattern;
            sf["spilot_boost"] = s.L1D_scattered_pilot_boost;
            sf["sbs_first"] = s.L1D_sbs_first;
            sf["sbs_last"] = s.L1D_sbs_last;
        }
        sf["freq_interleaver"] = s.L1D_frequency_interleaver;
        sf["plp_count"] = (int)s.plps.size();

        json plps = json::array();
        for (size_t j = 0; j < s.plps.size(); j++) {
            const auto& p = s.plps[j];
            json plp;
            plp["id"] = p.L1D_plp_id;
            plp["lls_flag"] = p.L1D_plp_lls_flag;
            plp["layer"] = p.L1D_plp_layer;
            plp["start"] = p.L1D_plp_start;
            plp["size"] = p.L1D_plp_size;
            plp["fec_type"] = p.L1D_plp_fec_type;
            plp["mod_order"] = p.L1D_plp_mod;
            plp["code_rate"] = p.L1D_plp_cod;
            plp["ti_mode"] = p.L1D_plp_TI_mode;
            plp["ti_extended"] = p.L1D_plp_TI_extended_interleaving;
            plp["cti_depth"] = p.L1D_plp_CTI_depth;
            plp["mimo_plp"] = p.L1D_plp_mimo;
            plp["stream_combining"] = p.L1D_plp_mimo_stream_combining;
            plp["iq_intervaling"] = p.L1D_plp_mimo_IQ_interleaving;
            plp["phase_hopping"] = p.L1D_plp_mimo_PH;
            plp["type"] = p.L1D_plp_type;
            plp["num_subslice"] = p.L1D_plp_num_subslices;
            plp["subslice_interval"] = p.L1D_plp_subslice_interval;
            plp["cell_intervaler"] = p.L1D_plp_HTI_cell_interleaver;
            plp["inter_subframe"] = p.L1D_plp_HTI_inter_subframe;
            plp["num_ti_blocks"] = p.L1D_plp_HTI_num_ti_blocks;
            plp["num_fec_blocks_max"] = p.L1D_plp_HTI_num_fec_blocks_max;
            plp["num_fec_blocks"] = p.L1D_plp_HTI_num_fec_blocks;
            plp["ldm_injection_level"] = p.L1D_plp_ldm_injection_level;
            plps.push_back(plp);
        }
        sf["plps"] = plps;
        subframes.push_back(sf);
    }

    response["config"] = cfg;
    response["subframes"] = subframes;

    return response;
}

// ============================================================================
// CNR Lookup
// ============================================================================

json JsonHandler::getCnrForPlp(int modOrder, int codeRateIdx, int fecType) {
    bool isLong = (fecType % 2 == 1);

    if (!isLong && modOrder > 3) {
        return nullptr;
    }

    using namespace ATSCTables;
    auto get = [&](const double longT[][12], int lMax, const double shortT[][12], int sMax) -> json {
        double v = getCnrValue(longT, lMax, shortT, sMax, modOrder, codeRateIdx, isLong);
        return std::isnan(v) ? json(nullptr) : json(v);
    };

    json cnr;
    cnr["bicm"]["awgn"] = get(BICM_AWGN_LONG, 6, BICM_AWGN_SHORT, 4);

    cnr["simulation"]["awgn"] = get(ANNEX_AWGN_SIM_LONG, 6, ANNEX_AWGN_SIM_SHORT, 4);
    cnr["simulation"]["rc20"] = get(ANNEX_RICIAN_SIM_LONG, 6, ANNEX_RICIAN_SIM_SHORT, 4);
    cnr["simulation"]["rl20"] = get(ANNEX_RAYLEIGH_SIM_LONG, 6, ANNEX_RAYLEIGH_SIM_SHORT, 4);

    cnr["lab"]["awgn"] = get(ANNEX_AWGN_LAB_LONG, 6, ANNEX_AWGN_LAB_SHORT, 4);
    cnr["lab"]["rc20"] = get(ANNEX_RICIAN_LAB_LONG, 6, ANNEX_RICIAN_LAB_SHORT, 4);
    cnr["lab"]["rl20"] = get(ANNEX_RAYLEIGH_LAB_LONG, 6, ANNEX_RAYLEIGH_LAB_SHORT, 4);

    cnr["field"]["awgn"] = get(ANNEX_AWGN_FIELD_LONG, 6, ANNEX_AWGN_FIELD_SHORT, 4);
    cnr["field"]["rc20"] = get(ANNEX_RICIAN_FIELD_LONG, 6, ANNEX_RICIAN_FIELD_SHORT, 4);
    cnr["field"]["rl20"] = get(ANNEX_RAYLEIGH_FIELD_LONG, 6, ANNEX_RAYLEIGH_FIELD_SHORT, 4);

    return cnr;
}

// ============================================================================
// Compute Results Data (all calculated data for display)
// ============================================================================

json JsonHandler::handleComputeResultsData(const json& req) {
    // This computes all results data that the frontend needs to display
    // including frame duration, preamble cells, PLP breakdown with bitrates/CNR
    json response;
    response["status"] = "ok";

    // Frame duration
    json frameDuration = handleGetFrameDuration(req);
    response["frameDuration"] = frameDuration;

    // Frame 2 log data
    json frame2Log = handleGetFrame2LogData(req);
    response["frame2LogData"] = frame2Log;

    // L1D size bytes
    json l1dSize = handleGetL1dSizeBytes(req);
    response["l1dSizeBytes"] = l1dSize;

    // Preamble fields (need preamble params from request)
    if (req.contains("preambleConfig")) {
        json preambleConfig = req["preambleConfig"];
        json preambleFields = computePreambleFields(
            preambleConfig.value("fftValue", 0),
            preambleConfig.value("giValue", 5),
            preambleConfig.value("preambleReducedCarriers", 0),
            preambleConfig.value("preambleStructure", 0),
            preambleConfig.value("l1dFecMode", 0),
            preambleConfig.value("l1dBytes", 25)
        );
        response["preambleFields"] = preambleFields;
    }

    // Per-subframe results
    if (req.contains("subframesData")) {
        json subframeResults = json::array();
        double totalDurationMs = 0.0;
        try {
            if (frameDuration.contains("totalDurationMs") && !frameDuration["totalDurationMs"].is_null()) {
                totalDurationMs = frameDuration["totalDurationMs"].get<double>();
            }
        } catch (...) {}

        // Compute frame duration in seconds for bitrate calculation
        double frameDurSec = totalDurationMs / 1000.0;

        for (const auto& sfData : req["subframesData"]) {
            json sfResult;

            // Compute PLP capacity for this subframe
            bool sbsFirst = false, sbsLast = false;
            if (sfData.contains("sbsFirst")) {
                sbsFirst = sfData["sbsFirst"].is_boolean() ? sfData["sbsFirst"].get<bool>() : (sfData["sbsFirst"].get<int>() != 0);
            }
            if (sfData.contains("sbsLast")) {
                sbsLast = sfData["sbsLast"].is_boolean() ? sfData["sbsLast"].get<bool>() : (sfData["sbsLast"].get<int>() != 0);
            }
            json plpCapacity = computePlpCapacity(
                sfData.value("fft", std::string("8K")),
                sfData.value("cred", 0),
                sfData.value("spLabelText", std::string("8")),
                sfData.value("spBoost", 0),
                sfData.value("numSymbols", 0),
                sbsFirst,
                sbsLast
            );

            sfResult["plpCapacity"] = plpCapacity;

            // Compute per-PLP data
            if (sfData.contains("plps")) {
                json plpResults = json::array();
                int subframeIndex = sfData.value("subframeIndex", 0);

                // Get subframe duration from frame_duration data
                double sfDurMs = 0;
                try {
                    if (frameDuration.contains("subframeDurations")) {
                        std::string sfKey = std::to_string(subframeIndex);
                        if (frameDuration["subframeDurations"].contains(sfKey) &&
                            !frameDuration["subframeDurations"][sfKey].is_null()) {
                            sfDurMs = frameDuration["subframeDurations"][sfKey].get<double>();
                        }
                    }
                } catch (...) {}
                double sfDurSec = sfDurMs / 1000.0;

                for (const auto& plpData : sfData["plps"]) {
                    json plpResult;

                    auto safeInt = [](const json& j, const std::string& key, int def) -> int {
                        if (!j.contains(key) || j[key].is_null()) return def;
                        return j[key].get<int>();
                    };

                    int fecType = safeInt(plpData, "fec_type", 0);
                    int modOrder = safeInt(plpData, "mod_order", 0);
                    int codeRateIdx = safeInt(plpData, "code_rate", 0);
                    int plpSize = safeInt(plpData, "size", 0);
                    int tiMode = safeInt(plpData, "ti_mode", 0);
                    int ctiDepthIdx = safeInt(plpData, "cti_depth", 0);
                    int tiExtended = safeInt(plpData, "ti_extended", 0);
                    int numFecBlocksMax = safeInt(plpData, "num_fec_blocks_max", 0);

                    int ldpcSize = (fecType % 2 == 1) ? 64800 : 16200;
                    int bitsPerCell = (modOrder + 1) * 2;
                    int fecBlockSize = ldpcSize / bitsPerCell;
                    int numFecBlocks = (plpSize > 0) ? (plpSize / fecBlockSize) : 0;

                    int codeRateNum = codeRateIdx + 2;
                    int codeRateDen = 15;

                    // TI cells calculation
                    int ctiDepthValues[] = {512, 724, 887, 1024};
                    int ctiDepthValuesExtended[] = {512, 724, 1254, 1448};
                    int celulasTI;
                    if (tiMode == 0) {
                        celulasTI = 512 * 511 / 2;
                    } else if (tiMode == 1) {
                        int* depthVals = tiExtended ? ctiDepthValuesExtended : ctiDepthValues;
                        int nRows = (ctiDepthIdx >= 0 && ctiDepthIdx < 4) ? depthVals[ctiDepthIdx] : 512;
                        celulasTI = nRows * (nRows - 1) / 2;
                    } else {
                        celulasTI = numFecBlocksMax * fecBlockSize;
                    }

                    // Outer code parity
                    int outerCodeParity = 0;
                    if (fecType == 0) outerCodeParity = 168;
                    else if (fecType == 1) outerCodeParity = 192;
                    else if (fecType == 2 || fecType == 3) outerCodeParity = 32;

                    int bbpHeader = 16;
                    double kLdpc = ldpcSize * ((double)codeRateNum / codeRateDen);
                    double kPayload = kLdpc - outerCodeParity - bbpHeader;
                    double payloadEff = (kLdpc > 0) ? (kPayload / kLdpc) : 1.0;

                    // BBFramerate and bitrate
                    double bbFrameRate = 0;
                    double bitrateMbps = 0;
                    if (sfDurSec > 0 && plpSize > 0) {
                        bbFrameRate = (double)numFecBlocks / sfDurSec;
                        double bitrateDiv = (frameDurSec > 0) ? frameDurSec : sfDurSec;
                        double bitrateBps = plpSize * bitsPerCell * ((double)codeRateNum / codeRateDen) * payloadEff / bitrateDiv;
                        bitrateMbps = bitrateBps / 1e6;
                    }

                    plpResult["fecBlockSize"] = fecBlockSize;
                    plpResult["numFecBlocks"] = numFecBlocks;
                    plpResult["celulasTI"] = celulasTI;
                    plpResult["bbFrameRate"] = (int)bbFrameRate;
                    plpResult["bitrateMbps"] = round(bitrateMbps * 1000.0) / 1000.0;
                    plpResult["bitsPerCell"] = bitsPerCell;
                    plpResult["ldpcSize"] = ldpcSize;

                    // CNR values
                    json cnr = getCnrForPlp(modOrder, codeRateIdx, fecType);
                    plpResult["cnr"] = cnr;

                    plpResults.push_back(plpResult);
                }

                sfResult["plpResults"] = plpResults;
            }

            subframeResults.push_back(sfResult);
        }

        response["subframeResults"] = subframeResults;
    }

    return response;
}
