#ifndef VALIDATION_H
#define VALIDATION_H

#include <vector>
#include <QString>

namespace Validation {
    std::vector<int> getAllowedPilotPatterns(int fft_size, int guard_interval);
    std::vector<int> getValidCodeRates(int fec_type, int mod_order);
    bool isValidModCod(int fec_type, int mod_order, int code_rate);
    QString fecTypeName(int v);
    QString modName(int v);
    QString codeRateName(int v);
}

#endif
