#ifndef JSON_HANDLER_H
#define JSON_HANDLER_H

#include "atsc_config.h"
#include "l1_calculator.h"
#include "l1_generator.h"
#include "l1_interpreter_class.h"
#include "nlohmann/json.hpp"
#include <string>

using json = nlohmann::json;

class JsonHandler {
public:
    JsonHandler(ATSC_Config& config, L1_Calculator& calculator,
                L1_Generator& generator, L1_Interpreter& interpreter);

    std::string processRequest(const std::string& jsonStr);

private:
    ATSC_Config& config;
    L1_Calculator& calculator;
    L1_Generator& generator;
    L1_Interpreter& interpreter;

    json handleCalculatePreambleFields(const json& req);
    json handleComputePlpCapacity(const json& req);
    json handleFillPlpSize(const json& req);
    json handleBatchFillAllPlps(const json& req);
    json handleGetL1dSizeBytes(const json& req);
    json handleGetFrame2LogData(const json& req);
    json handleGetFrameDuration(const json& req);
    json handleGetValidationTables(const json& req);
    json handleSetConfig(const json& req);
    json handleGetConfig(const json& req);
    json handleComputeResultsData(const json& req);

    // Helper methods
    json computePreambleFields(int fftValue, int giValue, int preambleReducedCarriers,
                               int preambleStructure, int l1dFecMode, int l1dBytes);
    json computePlpCapacity(const std::string& fft, int cred, const std::string& spLabelText,
                            int spBoost, int numSymbols, bool sbsFirst, bool sbsLast);
    json getCnrForPlp(int modOrder, int codeRateIdx, int fecType);

    void applyConfigFromJson(const json& req);
    void regenerateAfterConfig();
};

#endif
