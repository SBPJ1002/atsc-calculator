#include "l1_calculator.h"
#include <chrono>
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <cstdint>

// ============================================================================
// Constants
// ============================================================================

static const int TIME_OFFSET_MODULO = 6912;
static const int LDPC_SIZE_LONG = 64800;
static const int LDPC_SIZE_SHORT = 16200;

// ============================================================================
// Static const lookup tables (file-scope, not class members)
// ============================================================================

static const int SBS_TOTAL_DATA_CELLS[3][5][16] = {

    {

        {4560, 4560, 5136, 5136, 5712, 5712, 6000, 6000, 6288, 6288, 6432, 6432, 6576, 6576, 6648, 6648},
        {4496, 4496, 5064, 5064, 5632, 5632, 5916, 5916, 6204, 6204, 6348, 6348, 6492, 6492, 6564, 6564},
        {4433, 4433, 4993, 4993, 5553, 5553, 5833, 5833, 6121, 6121, 6265, 6265, 6409, 6409, 6481, 6481},
        {4370, 4370, 4922, 4922, 5474, 5474, 5750, 5750, 6038, 6038, 6182, 6182, 6326, 6326, 6398, 6398},
        {4307, 4307, 4851, 4851, 5395, 5395, 5667, 5667, 5955, 5955, 6099, 6099, 6243, 6243, 6315, 6315},
    },

    {
        {9120, 9120, 10272, 10272, 11424, 11424, 12000, 12000, 12576, 12576, 12864, 12864, 13152, 13152, 13296, 13296},
        {8992, 8992, 10128, 10128, 11264, 11264, 11832, 11832, 12408, 12408, 12696, 12696, 12984, 12984, 13128, 13128},
        {8867, 8867, 9987,  9987,  11107, 11107, 11667, 11667, 12243, 12243, 12531, 12531, 12819, 12819, 12963, 12963},
        {8740, 8740, 9844,  9844,  10948, 10948, 11500, 11500, 12076, 12076, 12364, 12364, 12652, 12652, 12796, 12796},
        {8614, 8614, 9702,  9702,  10790, 10790, 11334, 11334, 11910, 11910, 12198, 12198, 12486, 12486, 12630, 12630},
    },

    {
        {18240, 18240, 0, 0, 22848, 22848, 24000, 24000, 25152, 25152, 25728, 25728, 26304, 26304, 26592, 26592},
        {17984, 17984, 0, 0, 22528, 22528, 23664, 23664, 24816, 24816, 25392, 25392, 25968, 25968, 26256, 26256},
        {17734, 17734, 0, 0, 22214, 22214, 23334, 23334, 24486, 24486, 25062, 25062, 25638, 25638, 25926, 25926},
        {17480, 17480, 0, 0, 21896, 21896, 23000, 23000, 24152, 24152, 24728, 24728, 25304, 25304, 25592, 25592},
        {17228, 17228, 0, 0, 21580, 21580, 22668, 22668, 23820, 23820, 24396, 24396, 24972, 24972, 25260, 25260},
    }
};

static const int SBS_ACTIVE_CELLS_CRED0[3][5][16] = {

    {

        {4560, 4560, 5136, 5136, 5712, 5712, 6000, 6000, 6288, 6288, 6432, 6432, 6576, 6576, 6648, 6648},

        {4560, 3904, 5009, 4332, 5456, 4856, 5716, 5168, 5976, 5508, 6132, 5691, 6297, 5922, 6384, 6064},

        {4123, 2922, 4600, 3467, 5114, 4147, 5398, 4558, 5729, 5010, 5919, 5252, 6123, 5564, 6231, 5757},

        {3801, 2148, 4278, 2868, 4843, 3588, 5188, 4078, 5533, 4616, 5751, 4906, 5986, 5282, 6125, 5515},

        {3467, 1534, 4022, 2245, 4629, 3146, 4971, 3697, 5379, 4305, 5618, 4633, 5877, 5058, 6015, 5324},
    },

    {

        {9120, 9120, 10272, 10272, 11424, 11424, 12000, 12000, 12576, 12576, 12864, 12864, 13152, 13152, 13296, 13296},

        {9120, 7807, 10017, 8663, 10912, 9708, 11431, 10331, 11950, 11011, 12262, 11374, 12593, 11834, 12766, 12116},

        {8244, 5841, 9199, 6930, 10225, 8288, 10793, 9109, 11455, 10010, 11835, 10493, 12243, 11113, 12458, 11497},

        {7601, 4290, 8554, 5731, 9684, 7168, 10375, 8146, 11064, 9221, 11499, 9798, 11968, 10544, 12245, 11008},

        {6933, 3063, 8043, 4484, 9256, 6282, 9939, 7383, 10755, 8596, 11233, 9248, 11750, 10094, 12024, 10622},
    },

    {

        {18240, 18240, 0, 0, 22848, 22848, 24000, 24000, 25152, 25152, 25728, 25728, 26304, 26304, 26592, 26592},

        {18240, 15612, 0, 0, 21823, 19412, 22861, 20658, 23899, 22017, 24523, 22742, 25185, 23661, 25532, 24228},

        {16488, 11678, 0, 0, 20449, 16570, 21585, 18212, 22908, 20011, 23668, 20976, 24483, 22215, 24913, 22982},

        {15202, 8576, 0, 0, 19367, 14329, 20747, 16283, 22126, 18431, 22995, 19582, 23933, 21073, 24487, 21998},

        {13865, 6121, 0, 0, 18510, 12555, 19876, 14755, 21507, 17178, 22463, 18479, 23497, 20170, 24045, 21225},
    }
};

static const int SBS_ACTIVE_CELLS_CRED1[3][5][16] = {

    {
        {4496, 4496, 5064, 5064, 5632, 5632, 5916, 5916, 6204, 6204, 6348, 6348, 6492, 6492, 6564, 6564},
        {4496, 3852, 4939, 4273, 5380, 4791, 5636, 5097, 5896, 5438, 6050, 5620, 6214, 5850, 6300, 5990},
        {4069, 2884, 4538, 3422, 5046, 4094, 5326, 4499, 5655, 4945, 5841, 5184, 6043, 5494, 6150, 5683},
        {3751, 2120, 4221, 2829, 4779, 3542, 5119, 4025, 5463, 4555, 5676, 4843, 5906, 5215, 6045, 5439},
        {3421, 1514, 3968, 2215, 4567, 3105, 4905, 3648, 5312, 4249, 5544, 4574, 5796, 4994, 5936, 5254},
    },

    {
        {8992, 8992, 10128, 10128, 11264, 11264, 11832, 11832, 12408, 12408, 12696, 12696, 12984, 12984, 13128, 13128},
        {8992, 7703, 9877, 8545, 10760, 9579, 11271, 10189, 11791, 10871, 12099, 11232, 12427, 11690, 12598, 11967},
        {8135, 5763, 9075, 6840, 10089, 8182, 10649, 8990, 11308, 9879, 11679, 10354, 12077, 10973, 12289, 11346},
        {7501, 4234, 8441, 5655, 9553, 7074, 10235, 8038, 10918, 9098, 11345, 9668, 11804, 10407, 12078, 10865},
        {6841, 3022, 7936, 4424, 9132, 6198, 9807, 7284, 10609, 8481, 11081, 9126, 11590, 9960, 11859, 10488},
    },

    {
        {17984, 17984, 0, 0, 22528, 22528, 23664, 23664, 24816, 24816, 25392, 25392, 25968, 25968, 26256, 26256},
        {17984, 15393, 0, 0, 21519, 19141, 22541, 20373, 23581, 21736, 24197, 22457, 24853, 23373, 25196, 23929},
        {16260, 11517, 0, 0, 20177, 16350, 21296, 17973, 22613, 19749, 23355, 20698, 24152, 21931, 24575, 22692},
        {14992, 8460, 0, 0, 19105, 14140, 20468, 16066, 21834, 18185, 22688, 19322, 23607, 20798, 24152, 21710},
        {13674, 6040, 0, 0, 18263, 12386, 19607, 14556, 21217, 16950, 22163, 18233, 23179, 19896, 23714, 20948},
    }
};

static const int SBS_ACTIVE_CELLS_CRED2[3][5][16] = {

    {
        {4433, 4433, 4993, 4993, 5553, 5553, 5833, 5833, 6121, 6121, 6265, 6265, 6409, 6409, 6481, 6481},
        {4433, 3800, 4869, 4214, 5304, 4726, 5557, 5027, 5817, 5368, 5969, 5549, 6132, 5778, 6217, 5916},
        {4013, 2846, 4477, 3378, 4978, 4040, 5255, 4441, 5581, 4880, 5764, 5116, 5964, 5424, 6069, 5608},
        {3699, 2092, 4164, 2790, 4715, 3495, 5050, 3972, 5393, 4494, 5601, 4780, 5827, 5147, 5966, 5364},
        {3374, 1493, 3914, 2184, 4506, 3063, 4838, 3598, 5245, 4192, 5471, 4514, 5717, 4929, 5858, 5180},
    },

    {
        {8867, 8867, 9987, 9987, 11107, 11107, 11667, 11667, 12243, 12243, 12531, 12531, 12819, 12819, 12963, 12963},
        {8867, 7599, 9739, 8427, 10609, 9449, 11114, 10048, 11634, 10731, 11938, 11090, 12262, 11545, 12431, 11820},
        {8025, 5687, 8953, 6751, 9956, 8075, 10509, 8873, 11162, 9752, 11527, 10218, 11913, 10831, 12122, 11199},
        {7399, 4178, 8328, 5580, 9423, 6981, 10095, 7929, 10774, 8976, 11192, 9538, 11643, 10262, 11912, 10724},
        {6746, 2982, 7827, 4365, 9003, 6115, 9666, 7184, 10465, 8367, 10927, 9005, 11432, 9825, 11693, 10355},
    },

    {
        {17734, 17734, 0, 0, 22214, 22214, 23334, 23334, 24486, 24486, 25062, 25062, 25638, 25638, 25926, 25926},
        {17734, 15178, 0, 0, 21217, 18872, 22227, 20089, 23267, 21457, 23876, 22173, 24524, 23089, 24863, 23633},
        {16039, 11359, 0, 0, 19910, 16133, 21015, 17740, 22324, 19495, 23052, 20426, 23826, 21655, 24242, 22408},
        {14788, 8345, 0, 0, 18847, 13954, 20190, 15854, 21547, 17947, 22385, 19069, 23285, 20530, 23820, 21426},
        {13488, 5959, 0, 0, 18019, 12220, 19339, 14361, 20930, 16727, 21867, 17994, 22865, 19629, 23387, 20675},
    }
};

static const int SBS_ACTIVE_CELLS_CRED3[3][5][16] = {

    {
        {4370, 4370, 4922, 4922, 5474, 5474, 5750, 5750, 6038, 6038, 6182, 6182, 6326, 6326, 6398, 6398},
        {4370, 3747, 4800, 4155, 5229, 4661, 5479, 4957, 5739, 5298, 5889, 5478, 6052, 5706, 6136, 5843},
        {3956, 2806, 4415, 3333, 4911, 3988, 5184, 4383, 5508, 4816, 5688, 5049, 5885, 5355, 5988, 5534},
        {3647, 2062, 4107, 2752, 4652, 3449, 4982, 3919, 5323, 4434, 5527, 4717, 5749, 5078, 5886, 5288},
        {3327, 1472, 3860, 2153, 4445, 3022, 4773, 3549, 5178, 4135, 5398, 4455, 5639, 4863, 5780, 5104},
    },

    {
        {8740, 8740, 9844, 9844, 10948, 10948, 11500, 11500, 12076, 12076, 12364, 12364, 12652, 12652, 12796, 12796},
        {8740, 7495, 9600, 8309, 10458, 9319, 10957, 9908, 11477, 10591, 11777, 10948, 12100, 11401, 12269, 11673},
        {7914, 5610, 8830, 6661, 9821, 7967, 10367, 8755, 11015, 9624, 11374, 10082, 11749, 10690, 11955, 11051},
        {7297, 4122, 8215, 5505, 9292, 6887, 9956, 7821, 10630, 8853, 11038, 9407, 11481, 10117, 11745, 10582},
        {6653, 2943, 7719, 4306, 8874, 6032, 9526, 7085, 10322, 8253, 10774, 8882, 11275, 9691, 11528, 10218},
    },

    {
        {17480, 17480, 0, 0, 21896, 21896, 23000, 23000, 24152, 24152, 24728, 24728, 25304, 25304, 25592, 25592},
        {17480, 14962, 0, 0, 20915, 18600, 21910, 19803, 22951, 21176, 23553, 21889, 24194, 22803, 24529, 23336},
        {15815, 11199, 0, 0, 19641, 15914, 20732, 17505, 22034, 19239, 22747, 20154, 23499, 21378, 23908, 22121},
        {14582, 8228, 0, 0, 18587, 13766, 19910, 15640, 21259, 17706, 22081, 18814, 22962, 20261, 23487, 21140},
        {13300, 5876, 0, 0, 17773, 12053, 19070, 14165, 20642, 16502, 21570, 17754, 22550, 19361, 23059, 20401},
    }
};

static const int SBS_ACTIVE_CELLS_CRED4[3][5][16] = {

    {
        {4307, 4307, 4851, 4851, 5395, 5395, 5667, 5667, 5955, 5955, 6099, 6099, 6243, 6243, 6315, 6315},
        {4307, 3695, 4731, 4096, 5154, 4596, 5401, 4887, 5661, 5228, 5809, 5407, 5971, 5634, 6054, 5769},
        {3900, 2767, 4354, 3288, 4843, 3935, 5114, 4324, 5434, 4751, 5611, 4981, 5806, 5285, 5907, 5460},
        {3596, 2033, 4050, 2715, 4589, 3403, 4914, 3865, 5253, 4373, 5452, 4654, 5670, 5010, 5806, 5213},
        {3280, 1452, 3806, 2123, 4385, 2982, 4707, 3500, 5111, 4078, 5325, 4396, 5560, 4797, 5702, 5027},
    },

    {
        {8614, 8614, 9702, 9702, 10790, 10790, 11334, 11334, 11910, 11910, 12198, 12198, 12486, 12486, 12630, 12630},
        {8614, 7390, 9461, 8192, 10308, 9189, 10801, 9769, 11321, 10451, 11617, 10806, 11938, 11256, 12105, 11525},
        {7803, 5531, 8706, 6571, 9687, 7860, 10227, 8638, 10868, 9497, 11220, 9946, 11586, 10549, 11789, 10903},
        {7195, 4064, 8101, 5430, 9162, 6793, 9817, 7712, 10486, 8730, 10884, 9276, 11320, 9973, 11579, 10440},
        {6561, 2903, 7610, 4246, 8745, 5950, 9387, 6986, 10179, 8139, 10620, 8760, 11117, 9556, 11362, 10082},
    },

    {
        {17228, 17228, 0, 0, 21580, 21580, 22668, 22668, 23820, 23820, 24396, 24396, 24972, 24972, 25260, 25260},
        {17228, 14747, 0, 0, 20615, 18330, 21595, 19520, 22637, 20897, 23232, 21607, 23866, 22519, 24197, 23041},
        {15592, 11042, 0, 0, 19374, 15697, 20451, 17273, 21747, 18987, 22446, 19887, 23175, 21106, 23577, 21838},
        {14378, 8113, 0, 0, 18329, 13580, 19632, 15429, 20973, 17470, 21779, 18563, 22642, 19997, 23156, 20858},
        {13115, 5796, 0, 0, 17529, 11889, 18803, 13973, 20357, 16282, 21276, 17519, 22238, 19099, 22733, 20131},
    }
};

// ============================================================================
// Static helper functions (file-scope, don't need class access)
// ============================================================================

static int get_sbs_total_data_cells(int fft_size, int reduced_carriers, int sp_pattern) {
    if (fft_size < 0 || fft_size > 2) return 0;
    if (reduced_carriers < 0 || reduced_carriers > 4) reduced_carriers = 0;
    if (sp_pattern < 0 || sp_pattern > 15) return 0;

    return SBS_TOTAL_DATA_CELLS[fft_size][reduced_carriers][sp_pattern];
}

static int get_sbs_active_cells(int fft_size, int reduced_carriers, int sp_pattern, int sp_boost) {
    if (fft_size < 0 || fft_size > 2) return 0;
    if (reduced_carriers < 0 || reduced_carriers > 4) reduced_carriers = 0;
    if (sp_pattern < 0 || sp_pattern > 15) return 0;
    if (sp_boost < 0 || sp_boost > 4) sp_boost = 0;

    switch (reduced_carriers) {
        case 0:
            return SBS_ACTIVE_CELLS_CRED0[fft_size][sp_boost][sp_pattern];
        case 1:
            return SBS_ACTIVE_CELLS_CRED1[fft_size][sp_boost][sp_pattern];
        case 2:
            return SBS_ACTIVE_CELLS_CRED2[fft_size][sp_boost][sp_pattern];
        case 3:
            return SBS_ACTIVE_CELLS_CRED3[fft_size][sp_boost][sp_pattern];
        case 4:
            return SBS_ACTIVE_CELLS_CRED4[fft_size][sp_boost][sp_pattern];
        default:
            return SBS_ACTIVE_CELLS_CRED0[fft_size][sp_boost][sp_pattern];
    }
}


// ============================================================================
// Constructor
// ============================================================================

L1_Calculator::L1_Calculator(ATSC_Config& cfg)
    : config(cfg)
    , time_offset_params{0, 0, 0, 0, 0, false}
{
}

// ============================================================================
// Member helper functions
// ============================================================================

int L1_Calculator::get_fft_size_samples(int fft_index) {
    switch (fft_index) {
        case 0: return 8192;
        case 1: return 16384;
        case 2: return 32768;
        default:
            std::cerr << "WARNING: Invalid FFT index " << fft_index << ", using 8K" << std::endl;
            return 8192;
    }
}

int L1_Calculator::get_guard_interval_samples(int gi_index, int /*fft_samples*/) {
    static const int gi_absolute_samples[] = {
        0,
		192,
		384,
		512,
		768,
		1024,
		1536,
		2048,
		2432,
		3072,
		3648,
		4096,
		4864
    };

    if (gi_index >= 0 && gi_index <= 12) {
        return gi_absolute_samples[gi_index];
    }

    std::cerr << "WARNING: Invalid GI index " << gi_index << ", using 512" << std::endl;
    return 512;
}

int L1_Calculator::get_bits_per_cell(int mod_index) {
    switch (mod_index) {
        case 0:  return 2;
        case 1:  return 4;
        case 2:  return 6;
        case 3:  return 8;
        case 4:  return 10;
        case 5:  return 12;

        case 6:  return 2;
        case 7:  return 4;
        case 8:  return 6;
        case 9:  return 8;
        case 10: return 10;
        case 11: return 12;
        default:
            std::cerr << "WARNING: Invalid modulation index " << mod_index << ", using QPSK (2 bits)" << std::endl;
            return 2;
    }
}

int L1_Calculator::get_ldpc_size(int fec_type) {
    if (fec_type <= 5) {
        return LDPC_SIZE_LONG;
    }
    return LDPC_SIZE_SHORT;
}

uint8_t L1_Calculator::count_total_plps() {
    uint8_t total_plps = 0;

    for (size_t i = 0; i < config.preamble.sub_frames.size() && i < (size_t)config.preamble.L1B_num_subframes; i++) {
        total_plps += config.preamble.sub_frames[i].plps.size();
    }

    return total_plps;
}

// ============================================================================
// L1D time information
// ============================================================================

void L1_Calculator::fill_L1D_time_information() {
    auto now = std::chrono::system_clock::now();

    auto duration_since_epoch = now.time_since_epoch();
    auto nanoseconds_total = std::chrono::duration_cast<std::chrono::nanoseconds>(duration_since_epoch).count();

    uint64_t seconds_total = nanoseconds_total / 1000000000ULL;
    config.preamble.L1D_time_sec = static_cast<uint32_t>(seconds_total & 0xFFFFFFFF);

    uint64_t fractional_ns = nanoseconds_total % 1000000000ULL;

    config.preamble.L1D_time_msec = static_cast<uint16_t>(fractional_ns / 1000000ULL);
    config.preamble.L1D_time_usec = static_cast<uint16_t>((fractional_ns % 1000000ULL) / 1000ULL);
    config.preamble.L1D_time_nsec = static_cast<uint16_t>(fractional_ns % 1000ULL);

}

// ============================================================================
// L1D size calculation
// ============================================================================

int L1_Calculator::calculate_L1D_size_bits() {
    int totalBits = 0;

    totalBits += 4;
    totalBits += 3;

    for (int L1D_rf_id = 1; L1D_rf_id < config.preamble.L1D_num_rf; L1D_rf_id++) {
        totalBits += 16;
        totalBits += 3;
    }
    if (config.preamble.L1B_time_info_flag != 0) {
        totalBits += 32;
        totalBits += 10;

        if (config.preamble.L1B_time_info_flag != 1) {
            totalBits += 10;
            if (config.preamble.L1B_time_info_flag != 2) {
                totalBits += 10;
            }
        }
    }
    for(int i = 0; i < config.preamble.L1B_num_subframes; i++) {
        if (i > 0) {
            totalBits += 1;
            totalBits += 2;
            totalBits += 2;
            totalBits += 3;
            totalBits += 4;
            totalBits += 11;
            totalBits += 5;
            totalBits += 3;
            totalBits += 1;
            totalBits += 1;
        }
        if (config.preamble.L1B_num_subframes - 1 > 0) {
            totalBits += 1;
        }

        totalBits += 1;

        bool shouldAddSbsNullCells = false;
        if (i == 0) {
            shouldAddSbsNullCells = (config.preamble.sub_frames[0].L1B_first_sub_sbs_first ||
                                   config.preamble.sub_frames[0].L1B_first_sub_sbs_last);
        } else {
            shouldAddSbsNullCells = (config.preamble.sub_frames[i].L1D_sbs_first ||
                                   config.preamble.sub_frames[i].L1D_sbs_last);
        }

        if (shouldAddSbsNullCells) {
            totalBits += 13;
        }

        totalBits += 6;

        for(size_t j = 0; j < config.preamble.sub_frames[i].plps.size(); j++) {
            totalBits += 6;
            totalBits += 1;
            totalBits += 2;
            totalBits += 24;
            totalBits += 24;
            totalBits += 2;
            totalBits += 4;

            if(config.preamble.sub_frames[i].plps[j].L1D_plp_fec_type <= 5) {
                totalBits += 4;
                totalBits += 4;
            }

            totalBits += 2;

            if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 0) {
                totalBits += 15;
            } else if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) {
                totalBits += 22;
            }

            if(config.preamble.L1D_num_rf > 0) {
                totalBits += 3;
                if(config.preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded > 0) {
                    totalBits += 2;
                    totalBits += 3 * config.preamble.sub_frames[i].plps[j].L1D_plp_num_channel_bonded;
                }
            }

            bool isMimo = (i == 0 && config.preamble.sub_frames[0].L1B_first_sub_mimo == 1) ||
                         (i > 0 && config.preamble.sub_frames[i].L1D_mimo == 1);
            if(isMimo) {
                totalBits += 1;
                totalBits += 1;
                totalBits += 1;
            }

            if(config.preamble.sub_frames[i].plps[j].L1D_plp_layer == 0) {
                totalBits += 1;

                if(config.preamble.sub_frames[i].plps[j].L1D_plp_type == 1) {
                    totalBits += 14;
                    totalBits += 24;
                }

                if(((config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) ||
                    (config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 2)) &&
                   (config.preamble.sub_frames[i].plps[j].L1D_plp_mod == 0)) {
                    totalBits += 1;
                }

                if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 1) {
                    totalBits += 3;
                    totalBits += 11;
                } else if(config.preamble.sub_frames[i].plps[j].L1D_plp_TI_mode == 2) {
                    totalBits += 1;
                    totalBits += 4;
                    totalBits += 12;

                    if(config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_inter_subframe == 0) {
                        totalBits += 12;
                    } else {
                        totalBits += 12 * (config.preamble.sub_frames[i].plps[j].L1D_plp_HTI_num_fec_blocks + 1);
                    }
                    totalBits += 1;
                }
            } else {
                totalBits += 5;
            }
        }
    }

    if(config.preamble.L1D_version == 1){
    	totalBits += 16;
    }

    for(int i = 0; i < config.preamble.L1B_num_subframes; i++) {
        if(i > 0) {
            totalBits += 1;
        }

        bool shouldProcessMimo = (i == 0 && config.preamble.sub_frames[0].L1B_first_sub_mimo == 1 && config.preamble.sub_frames[0].L1B_first_sub_mimo_mixed == 1) ||
                                (i > 0 && config.preamble.sub_frames[i].L1D_mimo == 1 && config.preamble.sub_frames[i].L1D_mimo_mixed == 1);

        if(shouldProcessMimo) {
            for(size_t j = 0; j < config.preamble.sub_frames[i].plps.size(); j++) {
                totalBits += 1;
                if(config.preamble.sub_frames[i].plps[j].L1D_plp_mimo == 1) {
                    totalBits += 1;
                    totalBits += 1;
                    totalBits += 1;
                }
            }
        }
    }

    totalBits += 32;

    return totalBits;
}

int L1_Calculator::calculate_L1D_size_bytes() {
    const int MIN_L1D_BYTES = 25;

    int bits = calculate_L1D_size_bits();
    int bytes = (bits + 7) / 8;

    if (bytes < MIN_L1D_BYTES) {
        std::cout << "WARNING: L1D calculated with " << bytes << " bytes. Adjusting to the minimum of "
                  << MIN_L1D_BYTES << " bytes in accordance with ATSC 3.0 standard" << std::endl;
        bytes = MIN_L1D_BYTES;
    }

    return bytes;
}

// ============================================================================
// Time offset
// ============================================================================

uint16_t L1_Calculator::calc_L1B_time_offset(uint16_t bsr_coefficient, uint16_t l1d_time_usec, uint16_t l1d_time_nsec)
{
    const uint64_t fs_hz = 384000ULL * (static_cast<uint64_t>(bsr_coefficient) + 16ULL);
    const uint64_t delta_ns = static_cast<uint64_t>(l1d_time_usec) * 1000ULL + static_cast<uint64_t>(l1d_time_nsec);

    const uint64_t num = delta_ns * fs_hz + 500000000ULL;
    const uint64_t den = 1000000000ULL;
    const uint64_t offset_samples = num / den;

    return static_cast<uint16_t>(offset_samples);
}

void L1_Calculator::compute_and_set_L1B_time_offset()
{
    const uint16_t bsr = static_cast<uint16_t>(config.bootstrap.bsr_coefficient);
    config.preamble.L1B_time_offset = calc_L1B_time_offset(
        bsr,
        static_cast<uint16_t>(config.preamble.L1D_time_usec),
        static_cast<uint16_t>(config.preamble.L1D_time_nsec)
    );
}

// ============================================================================
// L1D cells estimation
// ============================================================================

uint32_t L1_Calculator::estimate_L1D_cells_from_regression(uint32_t l1d_size_bytes, uint8_t fec_mode)
{
    double x = std::max(25u, l1d_size_bytes);
    double cells;

    switch (fec_mode) {
        case 0: cells = 48.4968 * x + 1574.5629; break;
        case 1: cells = 12.0000 * x + 474.0000; break;
        case 2: cells = 107.6082 * sqrt(x) + 43.9439; break;
        case 3: cells = 3.8087 * x + 242.6673; break;
        case 4: cells = 2.3328 * x + 145.3357; break;
        case 5: cells = 1.6913 * x + 81.2802; break;
        case 6: cells = 1.1896 * x + 54.9410; break;
        default: cells = 1.1896 * x + 54.9410; break;
    }

    uint32_t result = static_cast<uint32_t>(std::round(cells));

    std::cout << "L1D Regression: bytes=" << l1d_size_bytes
              << ", FEC_mode=" << (int)fec_mode
              << " -> total_cells=" << result << std::endl;

    return result;
}

uint32_t L1_Calculator::calc_L1B_L1_Detail_total_cells(uint32_t l1d_size_bytes, uint8_t fec_mode, uint8_t additional_parity_mode)
{
    uint32_t total_cells = estimate_L1D_cells_from_regression(l1d_size_bytes, fec_mode);

    return total_cells;
}

void L1_Calculator::compute_and_set_L1B_L1_Detail_total_cells(uint8_t fec_mode)
{
    config.preamble.L1B_L1_Detail_total_cells = calc_L1B_L1_Detail_total_cells(
        static_cast<uint32_t>(config.preamble.L1B_L1_Detail_size_bytes),
        fec_mode,
        static_cast<uint8_t>(config.preamble.L1B_L1_Detail_additional_parity_mode)
    );
}

// ============================================================================
// SBS null cells
// ============================================================================

int L1_Calculator::calculate_sbs_null_cells(int subframe_index) {
    int fft_size, reduced_carriers, sp_pattern, sp_boost;
    bool sbs_first, sbs_last;

    if (subframe_index == 0) {
        fft_size = config.preamble.sub_frames[0].L1B_first_sub_fft_size;
        reduced_carriers = config.preamble.sub_frames[0].L1B_first_sub_reduced_carriers;
        sp_pattern = config.preamble.sub_frames[0].L1B_first_sub_scattered_pilot_pattern;
        sp_boost = config.preamble.sub_frames[0].L1B_first_sub_scattered_pilot_boost;
        sbs_first = config.preamble.sub_frames[0].L1B_first_sub_sbs_first != 0;
        sbs_last = config.preamble.sub_frames[0].L1B_first_sub_sbs_last != 0;
    } else {
        fft_size = config.preamble.sub_frames[subframe_index].L1D_fft_size;
        reduced_carriers = config.preamble.sub_frames[subframe_index].L1D_reduced_carriers;
        sp_pattern = config.preamble.sub_frames[subframe_index].L1D_scattered_pilot_pattern;
        sp_boost = config.preamble.sub_frames[subframe_index].L1D_scattered_pilot_boost;
        sbs_first = config.preamble.sub_frames[subframe_index].L1D_sbs_first != 0;
        sbs_last = config.preamble.sub_frames[subframe_index].L1D_sbs_last != 0;
    }

    if (!sbs_first && !sbs_last) {
        return 0;
    }

    int N_Data_B = get_sbs_total_data_cells(fft_size, reduced_carriers, sp_pattern);

    int N_C_B = get_sbs_active_cells(fft_size, reduced_carriers, sp_pattern, sp_boost);

    int sbs_null_cells = N_Data_B - N_C_B;

    std::cout << "SBS Null Cells Calculation (ATSC A/322 Annex F):" << std::endl;
    std::cout << "  FFT Size: " << (fft_size == 0 ? "8K" : (fft_size == 1 ? "16K" : "32K")) << std::endl;
    std::cout << "  Reduced Carriers (Cred): " << reduced_carriers << std::endl;
    std::cout << "  SP Pattern: " << sp_pattern << std::endl;
    std::cout << "  SP Boost Index: " << sp_boost << std::endl;
    std::cout << "  N_Data^B (Table 7.5/7.6): " << N_Data_B << std::endl;
    std::cout << "  N_C^B (Table F.1.x): " << N_C_B << std::endl;
    std::cout << "  N_Null^B = " << N_Data_B << " - " << N_C_B << " = " << sbs_null_cells << std::endl;

    if (sbs_null_cells < 0) {
        std::cout << "WARNING: sbs_null_cells calculated as negative (" << sbs_null_cells
                  << "). Setting to 0." << std::endl;
        sbs_null_cells = 0;
    }

    if (sbs_null_cells > 8191) {
        std::cout << "WARNING: sbs_null_cells exceeds 13 bits (" << sbs_null_cells
                  << "). Limiting to 8191." << std::endl;
        sbs_null_cells = 8191;
    }

    return sbs_null_cells;
}

void L1_Calculator::compute_and_set_all_sbs_null_cells() {
    for (int i = 0; i < config.preamble.L1B_num_subframes; i++) {
        config.preamble.sub_frames[i].L1D_sbs_null_cells = calculate_sbs_null_cells(i);
    }
}

// ============================================================================
// Multi-frame calculation helpers
// ============================================================================

void L1_Calculator::calculate_time_offset_params() {
    time_offset_params.fft_samples = get_fft_size_samples(
        config.preamble.sub_frames[0].L1B_first_sub_fft_size);

    time_offset_params.gi_samples = get_guard_interval_samples(
        config.preamble.sub_frames[0].L1B_first_sub_guard_interval,
        time_offset_params.fft_samples);

    time_offset_params.t_symbol = time_offset_params.fft_samples +
                                     time_offset_params.gi_samples;

    int num_ofdm_symbols = config.preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols;
    int preamble_symbols = config.preamble.L1B_preamble_num_symbols + 1;

    time_offset_params.n_total = num_ofdm_symbols + preamble_symbols;

    int64_t frame_duration_samples = (int64_t)time_offset_params.n_total *
                                      time_offset_params.t_symbol;
    time_offset_params.delta_t = frame_duration_samples % TIME_OFFSET_MODULO;

    time_offset_params.initialized = true;

    std::cout << "=== Time Offset Parameters ===" << std::endl;
    std::cout << "  FFT size: " << time_offset_params.fft_samples << " samples" << std::endl;
    std::cout << "  Guard interval: " << time_offset_params.gi_samples << " samples" << std::endl;
    std::cout << "  T_symbol: " << time_offset_params.t_symbol << " samples" << std::endl;
    std::cout << "  N_total (OFDM + preamble): " << time_offset_params.n_total << " symbols" << std::endl;
    std::cout << "  Delta_t: " << time_offset_params.delta_t << " samples" << std::endl;
}

void L1_Calculator::calculate_fec_block_start_params(int subframe_idx, int plp_idx) {
    int key = subframe_idx * 100 + plp_idx;

    FecBlockStartParams params;

    int ldpc_size    = get_ldpc_size(config.preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_fec_type);
    int bits_per_cell = get_bits_per_cell(config.preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_mod);

    params.c_block  = ldpc_size / bits_per_cell;
    params.plp_size = config.preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_size;

    if (params.plp_size == 0) {
        params.delta_fbs = 0;
    } else {
        int remainder    = params.plp_size % params.c_block;
        params.delta_fbs = (params.c_block - remainder) % params.c_block;
    }

    params.initialized = true;
    fec_params[key] = params;
}

uint16_t L1_Calculator::calculate_L1B_time_offset_for_frame(int frame_idx) {
    if (!time_offset_params.initialized) {
        calculate_time_offset_params();
    }

    int64_t time_offset = ((int64_t)frame_idx * time_offset_params.delta_t) % TIME_OFFSET_MODULO;

    return (uint16_t)time_offset;
}

int L1_Calculator::calculate_L1D_plp_fec_block_start_for_frame(int frame_idx, int subframe_idx, int plp_idx) {
    int key = subframe_idx * 100 + plp_idx;

    if (fec_params.find(key) == fec_params.end() || !fec_params[key].initialized) {
        calculate_fec_block_start_params(subframe_idx, plp_idx);
    }

    FecBlockStartParams& params = fec_params[key];

    std::cout << "  [DEBUG] plp_size=" << params.plp_size
              << " c_block=" << params.c_block
              << " delta_fbs=" << params.delta_fbs
              << " frame_idx=" << frame_idx
              << " subframe=" << subframe_idx
              << " plp=" << plp_idx << std::endl;

    if (params.plp_size == 0) {
        int ti_mode = config.preamble.sub_frames[subframe_idx].plps[plp_idx].L1D_plp_TI_mode;
        if (ti_mode == 1) {
            return (1 << 22) - 1;
        } else {
            return (1 << 15) - 1;
        }
    }

    int64_t fec_block_start = ((int64_t)frame_idx * params.delta_fbs) % params.c_block;

    return (int)fec_block_start;
}

void L1_Calculator::reset_variable_field_params() {
    time_offset_params.initialized = false;
    fec_params.clear();
}

void L1_Calculator::validate_and_adjust_preamble_num_symbols() {

    if (config.preamble.L1B_preamble_num_symbols < 0) {
        std::cout << "[VALIDATION] L1B_preamble_num_symbols=" << config.preamble.L1B_preamble_num_symbols
                  << " is below minimum. Adjusting to 0 (1 symbol)." << std::endl;
        config.preamble.L1B_preamble_num_symbols = 0;
    }
}

double L1_Calculator::calculate_frame_duration_ms() {

    double Fs = 384000.0 * (config.bootstrap.bsr_coefficient + 16);
    double Ts = 1.0 / Fs;

    double Fs_bs = config.bootstrap.system_bandwidth * 1024000.0;
    int bs_num_symbols = config.bootstrap.bootstrap_symbol + 1;
    int bs_total_samples = 2048 + (bs_num_symbols - 1) * (2048 + 512);
    double T_BS = bs_total_samples / Fs_bs;

    if (config.preamble.L1B_frame_length_mode == 0) {

        double duration_ms = config.preamble.L1B_frame_length * 5.0;
        std::cout << "=== Frame Duration (Time-Aligned) ===" << std::endl;
        std::cout << "  L1B_frame_length = " << config.preamble.L1B_frame_length << std::endl;
        std::cout << "  Duration = " << duration_ms << " ms" << std::endl;
        return duration_ms;
    }

    int preamble_num_symbols = config.preamble.L1B_preamble_num_symbols + 1;
    int preamble_fft = get_fft_size_samples(config.preamble.sub_frames[0].L1B_first_sub_fft_size);
    int preamble_gi = get_guard_interval_samples(config.preamble.sub_frames[0].L1B_first_sub_guard_interval, preamble_fft);
    double T_symbol_preamble = (preamble_fft + preamble_gi) * Ts;

    double duration_s = T_BS + preamble_num_symbols * T_symbol_preamble;

    std::cout << "=== Frame Duration (Symbol-Aligned) ===" << std::endl;
    std::cout << "  Fs = " << Fs << " Hz, Ts = " << Ts << " s" << std::endl;
    std::cout << "  Bootstrap: " << bs_num_symbols << " symbols, "
              << bs_total_samples << " samples @ " << Fs_bs / 1e6 << " MHz, T_BS = "
              << T_BS * 1000.0 << " ms" << std::endl;
    std::cout << "  Preamble: " << preamble_num_symbols << " symbols, T_symbol = "
              << T_symbol_preamble * 1000.0 << " ms" << std::endl;

    for (int i = 0; i < config.preamble.L1B_num_subframes; i++) {
        int fft_idx, gi_idx, num_ofdm;

        if (i == 0) {
            fft_idx = config.preamble.sub_frames[0].L1B_first_sub_fft_size;
            gi_idx = config.preamble.sub_frames[0].L1B_first_sub_guard_interval;
            num_ofdm = config.preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols;
        } else {
            fft_idx = config.preamble.sub_frames[i].L1D_fft_size;
            gi_idx = config.preamble.sub_frames[i].L1D_guard_interval;
            num_ofdm = config.preamble.sub_frames[i].L1D_num_ofdm_symbols;
        }

        int fft_samples = get_fft_size_samples(fft_idx);
        int gi_samples = get_guard_interval_samples(gi_idx, fft_samples);
        double T_symbol_sf = (fft_samples + gi_samples) * Ts;

        duration_s += num_ofdm * T_symbol_sf;

        std::cout << "  Subframe " << i << ": " << num_ofdm << " OFDM symbols, T_symbol = "
                  << T_symbol_sf * 1000.0 << " ms, subtotal = " << num_ofdm * T_symbol_sf * 1000.0 << " ms" << std::endl;
    }

    duration_s += config.preamble.L1B_additional_samples * Ts;

    double duration_ms = duration_s * 1000.0;
    std::cout << "  Additional samples: " << config.preamble.L1B_additional_samples << std::endl;
    std::cout << "  Total duration = " << duration_ms << " ms" << std::endl;

    return duration_ms;
}

void L1_Calculator::write_frame_duration_file(double total_duration_ms) {
    std::ofstream durationFile("config/log/frame_duration.txt");
    if (!durationFile.is_open()) {
        std::cerr << "ERROR: Failed to write config/log/frame_duration.txt" << std::endl;
        return;
    }

    durationFile << std::fixed << std::setprecision(6);
    durationFile << "frame_duration_ms=" << total_duration_ms << std::endl;

    double Fs = 384000.0 * (config.bootstrap.bsr_coefficient + 16);
    double Ts = 1.0 / Fs;

    for (int i = 0; i < config.preamble.L1B_num_subframes; i++) {
        int fft_idx, gi_idx, num_ofdm;

        if (i == 0) {
            fft_idx = config.preamble.sub_frames[0].L1B_first_sub_fft_size;
            gi_idx = config.preamble.sub_frames[0].L1B_first_sub_guard_interval;
            num_ofdm = config.preamble.sub_frames[0].L1B_first_sub_num_ofdm_symbols;
        } else {
            fft_idx = config.preamble.sub_frames[i].L1D_fft_size;
            gi_idx = config.preamble.sub_frames[i].L1D_guard_interval;
            num_ofdm = config.preamble.sub_frames[i].L1D_num_ofdm_symbols;
        }

        int fft_samples = get_fft_size_samples(fft_idx);
        int gi_samples = get_guard_interval_samples(gi_idx, fft_samples);
        double sf_duration_ms = num_ofdm * (fft_samples + gi_samples) * Ts * 1000.0;

        durationFile << "subframe_" << i << "_duration_ms=" << sf_duration_ms << std::endl;
    }

    durationFile.close();
    std::cout << "Saved frame duration: config/log/frame_duration.txt" << std::endl;
}
