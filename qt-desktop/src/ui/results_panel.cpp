#include "ui/results_panel.h"
#include "engine/calculator.h"
#include "engine/atsc_tables.h"
#include "engine/cnr_tables.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <cmath>

struct PreambleEntry {
    int giSamples;
    int cells[5];
};

static const PreambleEntry TABLE_7_2_8K[] = {
    { 192, {6432,6342,6253,6164,6075}},
    { 384, {6000,5916,5833,5750,5667}},
    { 512, {5712,5632,5553,5474,5395}},
    { 768, {5136,5064,4993,4922,4851}},
    {1024, {4560,4496,4433,4370,4307}},
    {1536, {5136,5064,4993,4922,4851}},
    {2048, {4560,4496,4433,4370,4307}},
    {   0, {0,0,0,0,0}}
};

static const PreambleEntry TABLE_7_2_16K[] = {
    { 192, {13296,13110,12927,12742,12558}},
    { 384, {12864,12684,12507,12328,12150}},
    { 512, {12576,12400,12227,12052,11878}},
    { 768, {12000,11832,11667,11500,11334}},
    {1024, {11424,11264,11107,10948,10790}},
    {1536, {10272,10128, 9987, 9844, 9702}},
    {2048, { 9120, 8992, 8867, 8740, 8614}},
    {2432, { 9120, 8992, 8867, 8740, 8614}},
    {3072, {10272,10128, 9987, 9844, 9702}},
    {3648, {10272,10128, 9987, 9844, 9702}},
    {4096, { 9120, 8992, 8867, 8740, 8614}},
    {   0, {0,0,0,0,0}}
};

static const PreambleEntry TABLE_7_2_32K[] = {
    { 192, {26592,26220,25854,25484,25116}},
    { 384, {26592,26220,25854,25484,25116}},
    { 512, {26304,25936,25574,25208,24844}},
    { 768, {25728,25368,25014,24656,24300}},
    {1024, {25152,24800,24454,24104,23756}},
    {1536, {24000,23664,23334,23000,22668}},
    {2048, {22848,22528,22214,21896,21580}},
    {2432, {22848,22528,22214,21896,21580}},
    {3072, {24000,23664,23334,23000,22668}},
    {3648, {24000,23664,23334,23000,22668}},
    {4096, {18240,17984,17734,17480,17228}},
    {4864, {18240,17984,17734,17480,17228}},
    {   0, {0,0,0,0,0}}
};

static int lookupPreambleCells(int fftSize, int giSamples, int cred) {
    const PreambleEntry* table = nullptr;
    switch (fftSize) {
        case 0: table = TABLE_7_2_8K; break;
        case 1: table = TABLE_7_2_16K; break;
        case 2: table = TABLE_7_2_32K; break;
        default: return 0;
    }
    int cr = std::max(0, std::min(4, cred));
    for (int i = 0; table[i].giSamples != 0; i++) {
        if (table[i].giSamples == giSamples)
            return table[i].cells[cr];
    }
    return 0;
}

static int getFftSamples(int fftIdx) {
    switch (fftIdx) {
        case 0: return 8192;
        case 1: return 16384;
        case 2: return 32768;
        default: return 8192;
    }
}

static int getGiSamplesFromIndex(int giIdx) {
    static const int vals[] = {0,192,384,512,768,1024,1536,2048,2432,3072,3648,4096,4864};
    if (giIdx >= 0 && giIdx <= 12) return vals[giIdx];
    return 512;
}

ResultsPanel::ResultsPanel(QWidget* parent) : QWidget(parent) {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* content = new QWidget;
    auto* mainLayout = new QVBoxLayout(content);
    mainLayout->setSpacing(14);
    mainLayout->setContentsMargins(14, 14, 14, 14);

    const QString valueStyle = "font-size: 13px; font-weight: bold; color: #58a6ff; min-width: 80px;";
    const QString labelStyle = "font-size: 12px; color: #8b949e;";

    auto* frameGroup = new QGroupBox("Frame Info");
    frameGroup->setMinimumWidth(280);
    auto* frameForm = new QFormLayout(frameGroup);
    frameForm->setSpacing(8);
    frameForm->setLabelAlignment(Qt::AlignLeft);
    m_frameDuration = new QLabel("--");
    m_frameDuration->setStyleSheet(valueStyle);
    m_frameDuration->setMinimumHeight(26);
    frameForm->addRow("Duration:", m_frameDuration);
    mainLayout->addWidget(frameGroup);

    auto* preambleGroup = new QGroupBox("Preamble");
    auto* preambleForm = new QFormLayout(preambleGroup);
    preambleForm->setSpacing(8);
    preambleForm->setLabelAlignment(Qt::AlignLeft);

    auto makeValueLabel = [&valueStyle]() {
        auto* l = new QLabel("--");
        l->setStyleSheet(valueStyle);
        l->setMinimumHeight(24);
        l->setTextInteractionFlags(Qt::TextSelectableByMouse);
        return l;
    };

    m_l1bCells = makeValueLabel();
    m_cellsFirst = makeValueLabel();
    m_cellsNext = makeValueLabel();
    m_numSymbols = makeValueLabel();
    m_l1dBytes = makeValueLabel();
    m_l1dCells = makeValueLabel();
    m_plpCells = makeValueLabel();

    preambleForm->addRow("L1B Cells:", m_l1bCells);
    preambleForm->addRow("Cells in First:", m_cellsFirst);
    preambleForm->addRow("Qtd. Symbols:", m_numSymbols);
    preambleForm->addRow("L1D Bytes:", m_l1dBytes);
    preambleForm->addRow("L1D Cells:", m_l1dCells);
    preambleForm->addRow("Cells in Next:", m_cellsNext);
    preambleForm->addRow("PLP Cells:", m_plpCells);

    m_timeOffset = makeValueLabel();
    preambleForm->addRow("Time Offset:", m_timeOffset);

    mainLayout->addWidget(preambleGroup);

    m_subframesContainer = new QWidget;
    m_subframesLayout = new QVBoxLayout(m_subframesContainer);
    m_subframesLayout->setContentsMargins(0, 0, 0, 0);
    m_subframesLayout->setSpacing(10);
    mainLayout->addWidget(m_subframesContainer);

    mainLayout->addStretch();
    scroll->setWidget(content);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->addWidget(scroll);
}

void ResultsPanel::clearLayout(QLayout* layout) {
    if (!layout) return;
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        } else if (item->layout()) {
            clearLayout(item->layout());
        }
        delete item;
    }
}

QString ResultsPanel::formatCnr(double v) {
    if (std::isnan(v)) return QString::fromUtf8("\u2014");
    return QString::number(v, 'f', 2);
}

FrameResult ResultsPanel::computeResults(const AtscConfig& config) {
    FrameResult result;
    const auto& p = config.preamble;
    const auto& b = config.bootstrap;

    result.l1bCells = AtscTables::L1B_CELLS_FIXED;
    result.l1dSizeBytes = Calculator::calculateL1DSizeBytes(config);

    int fftSize = 0;
    int giIdx = 0;
    if (!config.subframes.empty()) {
        fftSize = config.subframes[0].fft_size;
        giIdx = config.subframes[0].guard_interval;
    }
    int giSamples = getGiSamplesFromIndex(giIdx);

    result.cellsInFirst = lookupPreambleCells(fftSize, giSamples, 4);
    result.cellsInNext = lookupPreambleCells(fftSize, giSamples, p.l1b_preamble_reduced_carriers);

    result.l1dCells = Calculator::estimateL1DCells(result.l1dSizeBytes, p.l1b_detail_fec_type);

    int l1Total = result.l1bCells + result.l1dCells;
    result.numPreambleSymbols = (l1Total > result.cellsInFirst) ? 2 : 1;
    int totalPreambleCells = result.cellsInFirst + (result.numPreambleSymbols - 1) * result.cellsInNext;
    result.plpCells = std::max(0, totalPreambleCells - result.l1bCells - (int)result.l1dCells);

    result.timeOffset = Calculator::calcTimeOffset(b.bsr_coefficient, p.l1d_time_usec, p.l1d_time_nsec);

    double Fs = 384000.0 * (b.bsr_coefficient + 16);
    double Ts = 1.0 / Fs;

    double Fs_bs = b.system_bandwidth * 1024000.0;
    int bs_num_symbols = b.bootstrap_symbol + 1;
    int bs_total_samples = 2048 + (bs_num_symbols - 1) * (2048 + 512);
    double T_BS = bs_total_samples / Fs_bs;

    if (p.l1b_frame_length_mode == 0) {
        result.totalDurationMs = p.l1b_frame_length * 5.0;
    } else {
        int preamble_fft = getFftSamples(fftSize);
        int preamble_gi = getGiSamplesFromIndex(giIdx);
        double T_symbol_preamble = (preamble_fft + preamble_gi) * Ts;

        double duration_s = T_BS + (p.l1b_preamble_num_symbols + 1) * T_symbol_preamble;

        for (int i = 0; i < p.l1b_num_subframes && i < (int)config.subframes.size(); i++) {
            const auto& sf = config.subframes[i];
            int sf_fft = getFftSamples(sf.fft_size);
            int sf_gi = getGiSamplesFromIndex(sf.guard_interval);
            double T_symbol_sf = (sf_fft + sf_gi) * Ts;
            duration_s += sf.num_ofdm_symbols * T_symbol_sf;
        }

        duration_s += p.l1b_additional_samples * Ts;
        result.totalDurationMs = duration_s * 1000.0;
    }

    result.subframes.resize(config.subframes.size());
    for (int i = 0; i < (int)config.subframes.size(); i++) {
        const auto& sf = config.subframes[i];
        auto& sfr = result.subframes[i];

        int sf_fft = getFftSamples(sf.fft_size);
        int sf_gi = getGiSamplesFromIndex(sf.guard_interval);
        sfr.durationMs = sf.num_ofdm_symbols * (sf_fft + sf_gi) * Ts * 1000.0;

        int cellsPerSymbol = AtscTables::getCellsPerSymbol(sf.fft_size, sf.reduced_carriers, sf.scattered_pilot_pattern);
        int sbsActiveCells = AtscTables::getSbsActiveCells(sf.fft_size, sf.reduced_carriers, sf.scattered_pilot_pattern, sf.scattered_pilot_boost);

        int nDataSymbols = sf.num_ofdm_symbols;
        if (sf.sbs_first) nDataSymbols--;
        if (sf.sbs_last) nDataSymbols--;
        int nSbsSymbols = (sf.sbs_first ? 1 : 0) + (sf.sbs_last ? 1 : 0);

        sfr.cellsInDS = cellsPerSymbol;
        sfr.cellsInSBS = nSbsSymbols * sbsActiveCells;

        sfr.dataCells = (nDataSymbols * cellsPerSymbol) + (nSbsSymbols * sbsActiveCells);

        sfr.totalCapacity = sfr.dataCells + result.plpCells;

        int totalPlpUsed = 0;
        sfr.plps.resize(sf.plps.size());
        for (int j = 0; j < (int)sf.plps.size(); j++) {
            const auto& plp = sf.plps[j];
            auto& pr = sfr.plps[j];

            pr.plpId = plp.id;
            pr.numCells = plp.size;
            pr.startCell = plp.start;
            pr.fecBlockStart = plp.fec_block_start;
            totalPlpUsed += plp.size;

            int ldpcSize = (plp.fec_type % 2 == 1) ? 64800 : 16200;
            int bitsPerCell = (plp.mod + 1) * 2;
            pr.fecBlockSize = ldpcSize / bitsPerCell;
            pr.numFecBlocks = (plp.size > 0 && pr.fecBlockSize > 0) ? plp.size / pr.fecBlockSize : 0;

            static const int ctiDepthVals[] = {512, 724, 887, 1024};
            static const int ctiDepthValsExtended[] = {512, 724, 1254, 1448};

            if (plp.ti_mode == 0) {

                pr.tiCells = 512 * 511 / 2;
            } else if (plp.ti_mode == 1) {

                const int* depthValues = plp.ti_extended_interleaving ? ctiDepthValsExtended : ctiDepthVals;
                int depthIdx = std::max(0, std::min(3, plp.cti_depth));
                int depth = depthValues[depthIdx];
                pr.tiCells = depth * (depth - 1) / 2;
            } else {

                pr.tiCells = plp.hti_num_fec_blocks_max * pr.fecBlockSize;
            }

            double sfDurSec = sfr.durationMs / 1000.0;
            double frameDurSec = result.totalDurationMs / 1000.0;
            if (sfDurSec > 0 && frameDurSec > 0 && pr.numFecBlocks > 0) {
                pr.bbFrameRate = (int)(pr.numFecBlocks / sfDurSec);

                int codeRateNum = plp.cod + 2;
                int codeRateDen = 15;

                int outerCodeParity = 0;
                if (plp.fec_type == 0) outerCodeParity = 168;
                else if (plp.fec_type == 1) outerCodeParity = 192;
                else if (plp.fec_type == 2 || plp.fec_type == 3) outerCodeParity = 32;

                int bbpHeader = 16;
                double kLdpc = ldpcSize * ((double)codeRateNum / codeRateDen);
                double kPayload = kLdpc - outerCodeParity - bbpHeader;
                double payloadEff = (kLdpc > 0) ? (kPayload / kLdpc) : 1.0;

                double bitrateBps = plp.size * bitsPerCell * ((double)codeRateNum / codeRateDen) * payloadEff / frameDurSec;
                pr.bitrateMbps = bitrateBps / 1e6;
            }

            pr.cnr = CnrTables::getCnrValues(plp.mod, plp.cod, plp.fec_type);
        }

        sfr.dummyCells = std::max(0, sfr.totalCapacity - totalPlpUsed);
    }

    return result;
}

void ResultsPanel::updateResults(const AtscConfig& config) {
    FrameResult r = computeResults(config);

    const QString valueStyle = "font-size: 13px; font-weight: bold; color: #58a6ff;";

    m_frameDuration->setText(QString::number(r.totalDurationMs, 'f', 4) + " ms");

    m_l1bCells->setText(QString::number(r.l1bCells));
    m_cellsFirst->setText(QString::number(r.cellsInFirst));
    m_cellsNext->setText(QString::number(r.cellsInNext));
    m_numSymbols->setText(QString::number(r.numPreambleSymbols));
    m_l1dBytes->setText(QString::number(r.l1dSizeBytes));
    m_l1dCells->setText(QString::number(r.l1dCells));
    m_plpCells->setText(QString::number(r.plpCells));
    m_timeOffset->setText(QString::number(r.timeOffset));

    clearLayout(m_subframesLayout);

    for (int i = 0; i < (int)r.subframes.size(); i++) {
        const auto& sfr = r.subframes[i];
        auto* sfGroup = new QGroupBox(QString("Subframe %1").arg(i));
        auto* sfForm = new QFormLayout(sfGroup);
        sfForm->setSpacing(8);
        sfForm->setLabelAlignment(Qt::AlignLeft);

        auto makeVal = [&valueStyle](const QString& text) {
            auto* l = new QLabel(text);
            l->setStyleSheet(valueStyle);
            l->setMinimumHeight(24);
            l->setTextInteractionFlags(Qt::TextSelectableByMouse);
            return l;
        };

        sfForm->addRow(QString::fromUtf8("Duração:"), makeVal(QString::number(sfr.durationMs, 'f', 4) + " ms"));
        sfForm->addRow(QString::fromUtf8("Células Disponíveis:"), makeVal(QString::number(sfr.totalCapacity)));
        sfForm->addRow(QString::fromUtf8("Células em DS:"), makeVal(QString::number(sfr.cellsInDS)));
        sfForm->addRow(QString::fromUtf8("Células em SBS:"), makeVal(QString::number(sfr.cellsInSBS)));
        sfForm->addRow(QString::fromUtf8("Células Fictícias:"), makeVal(QString::number(sfr.dummyCells)));

        int totalUsed = 0;
        for (const auto& pr : sfr.plps) totalUsed += pr.numCells;
        double usedPercent = sfr.totalCapacity > 0 ? std::min(100.0, (double)totalUsed / sfr.totalCapacity * 100.0) : 0;
        bool isOverCapacity = totalUsed > sfr.totalCapacity;

        auto* progressBar = new QProgressBar;
        progressBar->setMinimum(0);
        progressBar->setMaximum(1000);
        progressBar->setValue((int)(usedPercent * 10));
        progressBar->setFormat(QString::number(usedPercent, 'f', 1) + "%");
        progressBar->setMinimumHeight(22);
        if (isOverCapacity) {
            progressBar->setStyleSheet(
                "QProgressBar { border: 1px solid #30363d; border-radius: 4px; background: #161b22; text-align: center; color: #e6edf3; font-size: 11px; }"
                "QProgressBar::chunk { background: #da3633; border-radius: 3px; }");
        } else {
            progressBar->setStyleSheet(
                "QProgressBar { border: 1px solid #30363d; border-radius: 4px; background: #161b22; text-align: center; color: #e6edf3; font-size: 11px; }"
                "QProgressBar::chunk { background: #238636; border-radius: 3px; }");
        }
        sfForm->addRow(progressBar);

        int remaining = sfr.totalCapacity - totalUsed;
        auto* infoLabel = new QLabel(
            QString("%1 usado  |  %2 total  |  %3 %4")
            .arg(totalUsed)
            .arg(sfr.totalCapacity)
            .arg(std::abs(remaining))
            .arg(isOverCapacity ? "excesso" : "livre")
        );
        infoLabel->setStyleSheet("font-size: 11px; color: #8b949e;");
        sfForm->addRow(infoLabel);

        for (int j = 0; j < (int)sfr.plps.size(); j++) {
            const auto& pr = sfr.plps[j];
            auto* plpGroup = new QGroupBox(QString("PLP %1").arg(pr.plpId));
            auto* plpForm = new QFormLayout(plpGroup);
            plpForm->setSpacing(6);
            plpForm->setLabelAlignment(Qt::AlignLeft);

            QString bbfrText = (pr.bbFrameRate > 0) ? QString::number(pr.bbFrameRate) + " fps" : QString::fromUtf8("\u2014");
            QString bitrateText = (pr.bitrateMbps > 0) ? QString::number(pr.bitrateMbps, 'f', 3) + " Mbps" : QString::fromUtf8("\u2014");

            plpForm->addRow("BBFramerate:", makeVal(bbfrText));
            plpForm->addRow("Bitrate:", makeVal(bitrateText));
            plpForm->addRow(QString::fromUtf8("Quantidade de Células:"), makeVal(QString::number(pr.numCells)));
            plpForm->addRow(QString::fromUtf8("Célula de Início:"), makeVal(QString::number(pr.startCell)));
            plpForm->addRow("FEC Block Start:", makeVal(QString::number(pr.fecBlockStart)));
            plpForm->addRow(QString::fromUtf8("Células TI:"), makeVal(QString::number(pr.tiCells)));
            plpForm->addRow("FEC Block Size:", makeVal(QString::number(pr.fecBlockSize) + " cells"));
            plpForm->addRow("#FEC Blocks:", makeVal(QString::number(pr.numFecBlocks)));

            if (pr.cnr.valid) {
                auto* cnrGroup = new QGroupBox("CNR / SNR");
                auto* cnrForm = new QFormLayout(cnrGroup);
                cnrForm->setSpacing(4);

                auto cnrVal = [this, &valueStyle](double v) {
                    auto* l = new QLabel(formatCnr(v) + " dB");
                    l->setStyleSheet(valueStyle);
                    l->setMinimumHeight(20);
                    return l;
                };

                auto* bicmLabel = new QLabel("BICM CNR");
                bicmLabel->setStyleSheet("font-weight: bold; color: #f0883e; font-size: 11px;");
                cnrForm->addRow(bicmLabel);
                cnrForm->addRow("  AWGN:", cnrVal(pr.cnr.bicm.awgn));

                auto* simLabel = new QLabel("Simulation CNR");
                simLabel->setStyleSheet("font-weight: bold; color: #f0883e; font-size: 11px;");
                cnrForm->addRow(simLabel);
                cnrForm->addRow("  AWGN:", cnrVal(pr.cnr.simulation.awgn));
                cnrForm->addRow("  RC20:", cnrVal(pr.cnr.simulation.rc20));
                cnrForm->addRow("  RL20:", cnrVal(pr.cnr.simulation.rl20));

                auto* labLabel = new QLabel("Lab CNR");
                labLabel->setStyleSheet("font-weight: bold; color: #f0883e; font-size: 11px;");
                cnrForm->addRow(labLabel);
                cnrForm->addRow("  AWGN:", cnrVal(pr.cnr.lab.awgn));
                cnrForm->addRow("  RC20:", cnrVal(pr.cnr.lab.rc20));
                cnrForm->addRow("  RL20:", cnrVal(pr.cnr.lab.rl20));

                auto* fieldLabel = new QLabel("Field CNR");
                fieldLabel->setStyleSheet("font-weight: bold; color: #f0883e; font-size: 11px;");
                cnrForm->addRow(fieldLabel);
                cnrForm->addRow("  AWGN:", cnrVal(pr.cnr.field.awgn));
                cnrForm->addRow("  RC20:", cnrVal(pr.cnr.field.rc20));
                cnrForm->addRow("  RL20:", cnrVal(pr.cnr.field.rl20));

                plpForm->addRow(cnrGroup);
            }

            sfForm->addRow(plpGroup);
        }

        m_subframesLayout->addWidget(sfGroup);
    }
}
