#ifndef ATSC_TABLES_H
#define ATSC_TABLES_H

#include <map>
#include <string>

namespace AtscTables {

const int L1B_CELLS_FIXED = 3820;

const std::map<int, int> GI_VALUE_TO_SAMPLES = {
    {1,192},{2,384},{3,512},{4,768},{5,1024},{6,1536},
    {7,2048},{8,2432},{9,3072},{10,3648},{11,4096},{12,4864}
};

const char* fftName(int v);
const char* spPatternName(int v);
int giSamples(int v);

extern const int TABLE_7_3_CELLS[3][5][8];

extern const int TABLE_7_4_CELLS[3][5][8];

extern const int SBS_TOTAL_DATA_CELLS[3][5][16];

extern const int SBS_ACTIVE_CELLS_CRED0[3][5][16];
extern const int SBS_ACTIVE_CELLS_CRED1[3][5][16];
extern const int SBS_ACTIVE_CELLS_CRED2[3][5][16];
extern const int SBS_ACTIVE_CELLS_CRED3[3][5][16];
extern const int SBS_ACTIVE_CELLS_CRED4[3][5][16];

int getCellsPerSymbol(int fft_size, int reduced_carriers, int sp_pattern);
int getSbsTotalDataCells(int fft_size, int reduced_carriers, int sp_pattern);
int getSbsActiveCells(int fft_size, int reduced_carriers, int sp_pattern, int sp_boost);

}

#endif
