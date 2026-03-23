#include "ui/plp_page.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>
#include <map>

PlpPage::PlpPage(QWidget* parent) : QWidget(parent) {
    m_tabs = new QTabWidget;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tabs);
}

QWidget* PlpPage::createPlpTab(int sfIdx, int plpIdx) {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* content = new QWidget;
    auto* form = new QFormLayout(content);
    form->setSpacing(8);
    form->setContentsMargins(16, 16, 16, 16);

    PlpWidgets w;

    w.layer = new QComboBox;
    w.layer->addItem("Core Layer", 0);
    w.layer->addItem("Enhanced Layer", 1);
    form->addRow("Layer:", w.layer);

    w.id = new QSpinBox;
    w.id->setRange(0, 63);
    form->addRow("PLP ID:", w.id);

    w.llsFlag = new QComboBox;
    w.llsFlag->addItem("OFF", 0);
    w.llsFlag->addItem("ON", 1);
    form->addRow("LLS Flag:", w.llsFlag);

    w.allocMode = new QComboBox;
    w.allocMode->addItem("Auto", 0);
    w.allocMode->addItem("Manual", 1);
    form->addRow("Start/Size:", w.allocMode);

    w.start = new QSpinBox;
    w.start->setRange(0, 999999999);
    w.start->setReadOnly(true);
    form->addRow("Start:", w.start);

    w.size = new QSpinBox;
    w.size->setRange(0, 999999999);
    w.size->setReadOnly(true);
    form->addRow("Size:", w.size);

    w.fecType = new QComboBox;
    w.fecType->addItem("BCH+16K LDPC", 0);
    w.fecType->addItem("BCH+64K LDPC", 1);
    w.fecType->addItem("CRC+16K LDPC", 2);
    w.fecType->addItem("CRC+64K LDPC", 3);
    w.fecType->addItem("16K LDPC only", 4);
    w.fecType->addItem("64K LDPC only", 5);
    form->addRow("FEC Type:", w.fecType);

    w.modOrder = new QComboBox;
    w.modOrder->addItem("QPSK", 0);
    w.modOrder->addItem("16QAM", 1);
    w.modOrder->addItem("64QAM", 2);
    w.modOrder->addItem("256QAM", 3);
    w.modOrder->addItem("1024QAM", 4);
    w.modOrder->addItem("4096QAM", 5);
    form->addRow("MOD Order:", w.modOrder);

    w.codeRate = new QComboBox;
    const char* crNames[] = {"2/15","3/15","4/15","5/15","6/15","7/15",
                             "8/15","9/15","10/15","11/15","12/15","13/15"};
    for (int i = 0; i < 12; i++)
        w.codeRate->addItem(crNames[i], i);
    form->addRow("Code Rate:", w.codeRate);

    w.fecBlockStart = new QSpinBox;
    w.fecBlockStart->setRange(0, 999999999);
    w.fecBlockStartRow = new QWidget;
    auto* fbsLayout = new QHBoxLayout(w.fecBlockStartRow);
    fbsLayout->setContentsMargins(0, 0, 0, 0);
    fbsLayout->addWidget(w.fecBlockStart);
    form->addRow("FEC Block Start:", w.fecBlockStartRow);

    w.tiMode = new QComboBox;
    w.tiMode->addItem("No TI", 0);
    w.tiMode->addItem("CTI", 1);
    w.tiMode->addItem("HTI", 2);
    form->addRow("TI Mode:", w.tiMode);

    w.ctiFecBlockStart = new QSpinBox;
    w.ctiFecBlockStart->setRange(0, 999999999);
    w.ctiFecBlockStartRow = new QWidget;
    auto* cfbsLayout = new QHBoxLayout(w.ctiFecBlockStartRow);
    cfbsLayout->setContentsMargins(0, 0, 0, 0);
    cfbsLayout->addWidget(w.ctiFecBlockStart);
    form->addRow("CTI FEC Block Start:", w.ctiFecBlockStartRow);

    w.tiExtended = new QComboBox;
    w.tiExtended->addItem("OFF", 0);
    w.tiExtended->addItem("ON", 1);
    w.tiExtendedRow = new QWidget;
    auto* teLayout = new QHBoxLayout(w.tiExtendedRow);
    teLayout->setContentsMargins(0, 0, 0, 0);
    teLayout->addWidget(w.tiExtended);
    form->addRow("TI Extended:", w.tiExtendedRow);

    w.ctiDepth = new QComboBox;
    w.ctiDepth->addItem("512", 0);
    w.ctiDepth->addItem("724", 1);
    w.ctiDepth->addItem("887", 2);
    w.ctiDepth->addItem("1024", 3);
    w.ctiDepthRow = new QWidget;
    auto* cdLayout = new QHBoxLayout(w.ctiDepthRow);
    cdLayout->setContentsMargins(0, 0, 0, 0);
    cdLayout->addWidget(w.ctiDepth);
    form->addRow("CTI Depth:", w.ctiDepthRow);

    w.cellInterleaver = new QComboBox;
    w.cellInterleaver->addItem("OFF", 0);
    w.cellInterleaver->addItem("ON", 1);
    w.cellInterleaverRow = new QWidget;
    auto* ciLayout = new QHBoxLayout(w.cellInterleaverRow);
    ciLayout->setContentsMargins(0, 0, 0, 0);
    ciLayout->addWidget(w.cellInterleaver);
    form->addRow("Cell Interleaver:", w.cellInterleaverRow);

    w.interSubframe = new QComboBox;
    w.interSubframe->addItem("Intra subframe HTI", 0);
    w.interSubframe->addItem("Inter subframe HTI", 1);
    w.interSubframeRow = new QWidget;
    auto* isLayout = new QHBoxLayout(w.interSubframeRow);
    isLayout->setContentsMargins(0, 0, 0, 0);
    isLayout->addWidget(w.interSubframe);
    form->addRow("Inter Subframe:", w.interSubframeRow);

    w.numTiBlocks = new QSpinBox;
    w.numTiBlocks->setRange(0, 65535);
    w.numTiBlocksRow = new QWidget;
    auto* ntbLayout = new QHBoxLayout(w.numTiBlocksRow);
    ntbLayout->setContentsMargins(0, 0, 0, 0);
    ntbLayout->addWidget(w.numTiBlocks);
    form->addRow("Num TI Blocks:", w.numTiBlocksRow);

    w.numFecBlocksMax = new QSpinBox;
    w.numFecBlocksMax->setRange(0, 65535);
    w.numFecBlocksMaxRow = new QWidget;
    auto* nfbmLayout = new QHBoxLayout(w.numFecBlocksMaxRow);
    nfbmLayout->setContentsMargins(0, 0, 0, 0);
    nfbmLayout->addWidget(w.numFecBlocksMax);
    form->addRow("Num FEC Blocks Max:", w.numFecBlocksMaxRow);

    w.numFecBlocks = new QSpinBox;
    w.numFecBlocks->setRange(0, 65535);
    w.numFecBlocksRow = new QWidget;
    auto* nfbLayout = new QHBoxLayout(w.numFecBlocksRow);
    nfbLayout->setContentsMargins(0, 0, 0, 0);
    nfbLayout->addWidget(w.numFecBlocks);
    form->addRow("Num FEC Blocks:", w.numFecBlocksRow);

    w.plpType = new QComboBox;
    w.plpType->addItem("Non-dispersed", 0);
    w.plpType->addItem("Dispersed", 1);
    w.plpTypeRow = new QWidget;
    auto* ptLayout = new QHBoxLayout(w.plpTypeRow);
    ptLayout->setContentsMargins(0, 0, 0, 0);
    ptLayout->addWidget(w.plpType);
    form->addRow("PLP Type:", w.plpTypeRow);

    w.numSubslice = new QSpinBox;
    w.numSubslice->setRange(0, 65535);
    w.numSubsliceRow = new QWidget;
    auto* nsLayout = new QHBoxLayout(w.numSubsliceRow);
    nsLayout->setContentsMargins(0, 0, 0, 0);
    nsLayout->addWidget(w.numSubslice);
    form->addRow("Num Subslice:", w.numSubsliceRow);

    w.subsliceInterval = new QSpinBox;
    w.subsliceInterval->setRange(0, 65535);
    w.subsliceIntervalRow = new QWidget;
    auto* siLayout = new QHBoxLayout(w.subsliceIntervalRow);
    siLayout->setContentsMargins(0, 0, 0, 0);
    siLayout->addWidget(w.subsliceInterval);
    form->addRow("Subslice Interval:", w.subsliceIntervalRow);

    w.ldmInjectionLevel = new QComboBox;
    for (int i = 0; i <= 30; i++) {
        double db = i * 0.5;
        if (i <= 10)
            db = i * 0.5;
        else
            db = 5.0 + (i - 10);
        w.ldmInjectionLevel->addItem(QString("%1 dB").arg(db, 0, 'f', 1), i);
    }
    w.ldmInjectionRow = new QWidget;
    auto* ldmLayout = new QHBoxLayout(w.ldmInjectionRow);
    ldmLayout->setContentsMargins(0, 0, 0, 0);
    ldmLayout->addWidget(w.ldmInjectionLevel);
    form->addRow("LDM Injection Level:", w.ldmInjectionRow);

    w.mimoPlp = new QComboBox;
    w.mimoPlp->addItem("OFF", 0);
    w.mimoPlp->addItem("ON", 1);
    w.mimoPlpRow = new QWidget;
    auto* mpLayout = new QHBoxLayout(w.mimoPlpRow);
    mpLayout->setContentsMargins(0, 0, 0, 0);
    mpLayout->addWidget(w.mimoPlp);
    form->addRow("MIMO PLP:", w.mimoPlpRow);

    w.streamCombining = new QComboBox;
    w.streamCombining->addItem("Disable", 0);
    w.streamCombining->addItem("Enable", 1);
    w.streamCombiningRow = new QWidget;
    auto* scLayout = new QHBoxLayout(w.streamCombiningRow);
    scLayout->setContentsMargins(0, 0, 0, 0);
    scLayout->addWidget(w.streamCombining);
    form->addRow("Stream Combining:", w.streamCombiningRow);

    w.iqInterleaving = new QComboBox;
    w.iqInterleaving->addItem("Disable", 0);
    w.iqInterleaving->addItem("Enable", 1);
    w.iqInterleavingRow = new QWidget;
    auto* iqLayout = new QHBoxLayout(w.iqInterleavingRow);
    iqLayout->setContentsMargins(0, 0, 0, 0);
    iqLayout->addWidget(w.iqInterleaving);
    form->addRow("IQ Interleaving:", w.iqInterleavingRow);

    w.phaseHopping = new QComboBox;
    w.phaseHopping->addItem("Disable", 0);
    w.phaseHopping->addItem("Enable", 1);
    w.phaseHoppingRow = new QWidget;
    auto* phLayout = new QHBoxLayout(w.phaseHoppingRow);
    phLayout->setContentsMargins(0, 0, 0, 0);
    phLayout->addWidget(w.phaseHopping);
    form->addRow("Phase Hopping:", w.phaseHoppingRow);

    connect(w.layer, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, idx = (int)m_widgets.size()](int) {
        if (idx < (int)m_widgets.size())
            updateFieldVisibility(m_widgets[idx]);
    });
    connect(w.tiMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, idx = (int)m_widgets.size()](int) {
        if (idx < (int)m_widgets.size())
            updateFieldVisibility(m_widgets[idx]);
    });
    connect(w.plpType, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, idx = (int)m_widgets.size()](int) {
        if (idx < (int)m_widgets.size())
            updateFieldVisibility(m_widgets[idx]);
    });
    connect(w.allocMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, idx = (int)m_widgets.size()](int) {
        if (idx < (int)m_widgets.size())
            updateAllocModeUi(m_widgets[idx]);
    });

    auto emitChanged = [this]() { emit configChanged(); };
    connect(w.layer, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.id, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.llsFlag, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.allocMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.start, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.size, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.fecType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.modOrder, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.codeRate, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.fecBlockStart, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.tiMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.ctiFecBlockStart, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.plpType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.ctiDepth, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.numFecBlocksMax, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.ldmInjectionLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.mimoPlp, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);

    m_widgets.push_back(w);
    updateFieldVisibility(m_widgets.back());

    scroll->setWidget(content);
    return scroll;
}

void PlpPage::updateFieldVisibility(PlpWidgets& w) {
    int layer = w.layer->currentData().toInt();
    int tiMode = w.tiMode->currentData().toInt();
    int plpType = w.plpType->currentData().toInt();

    bool isEnhanced = (layer == 1);

    w.ldmInjectionRow->setVisible(isEnhanced);
    w.plpTypeRow->setVisible(!isEnhanced);
    w.tiExtendedRow->setVisible(!isEnhanced);
    w.fecBlockStartRow->setVisible(!isEnhanced);

    bool showSubslice = !isEnhanced && (plpType == 1);
    w.numSubsliceRow->setVisible(showSubslice);
    w.subsliceIntervalRow->setVisible(showSubslice);

    bool showCti = !isEnhanced && (tiMode == 1);
    w.ctiDepthRow->setVisible(showCti);
    w.ctiFecBlockStartRow->setVisible(showCti);

    bool showHti = !isEnhanced && (tiMode == 2);
    w.cellInterleaverRow->setVisible(showHti);
    w.interSubframeRow->setVisible(showHti);
    w.numTiBlocksRow->setVisible(showHti);
    w.numFecBlocksMaxRow->setVisible(showHti);
    w.numFecBlocksRow->setVisible(showHti);

    if (isEnhanced) {
        w.ctiDepthRow->setVisible(false);
        w.ctiFecBlockStartRow->setVisible(false);
        w.cellInterleaverRow->setVisible(false);
        w.interSubframeRow->setVisible(false);
        w.numTiBlocksRow->setVisible(false);
        w.numFecBlocksMaxRow->setVisible(false);
        w.numFecBlocksRow->setVisible(false);
    }
}

void PlpPage::rebuildTabs(const AtscConfig& config) {
    while (m_tabs->count() > 0) m_tabs->removeTab(0);
    m_widgets.clear();
    m_tabMapping.clear();

    for (int sf = 0; sf < (int)config.subframes.size(); sf++) {
        int plpCount = std::max(1, (int)config.subframes[sf].plps.size());
        for (int p = 0; p < plpCount; p++) {
            m_tabMapping.push_back({sf, p});
            m_tabs->addTab(createPlpTab(sf, p),
                           QString("SF%1 PLP%2").arg(sf).arg(p));
        }
    }
}

void PlpPage::loadFromConfig(const AtscConfig& config) {
    if (m_widgets.size() != m_tabMapping.size()) return;

    for (int t = 0; t < (int)m_tabMapping.size(); t++) {
        auto [sf, p] = m_tabMapping[t];
        if (sf >= (int)config.subframes.size()) continue;
        if (p >= (int)config.subframes[sf].plps.size()) continue;

        const auto& plp = config.subframes[sf].plps[p];
        auto& w = m_widgets[t];

        w.layer->setCurrentIndex(plp.layer);
        w.id->setValue(plp.id);
        w.llsFlag->setCurrentIndex(plp.lls_flag);
        w.allocMode->setCurrentIndex(plp.alloc_mode);
        w.start->setValue(plp.start);
        w.size->setValue(plp.size);
        w.fecType->setCurrentIndex(plp.fec_type);
        w.modOrder->setCurrentIndex(plp.mod);
        w.codeRate->setCurrentIndex(plp.cod);
        w.fecBlockStart->setValue(plp.fec_block_start);
        w.tiMode->setCurrentIndex(plp.ti_mode);
        w.ctiFecBlockStart->setValue(plp.cti_fec_block_start);
        w.tiExtended->setCurrentIndex(plp.ti_extended_interleaving);
        w.ctiDepth->setCurrentIndex(plp.cti_depth);
        w.cellInterleaver->setCurrentIndex(plp.hti_cell_interleaver);
        w.interSubframe->setCurrentIndex(plp.hti_inter_subframe);
        w.numTiBlocks->setValue(plp.hti_num_ti_blocks);
        w.numFecBlocksMax->setValue(plp.hti_num_fec_blocks_max);
        w.numFecBlocks->setValue(plp.hti_num_fec_blocks);
        w.plpType->setCurrentIndex(plp.type);
        w.numSubslice->setValue(plp.num_subslices);
        w.subsliceInterval->setValue(plp.subslice_interval);
        w.ldmInjectionLevel->setCurrentIndex(plp.ldm_injection_level);
        w.mimoPlp->setCurrentIndex(plp.mimo);
        w.streamCombining->setCurrentIndex(plp.mimo_stream_combining);
        w.iqInterleaving->setCurrentIndex(plp.mimo_iq_interleaving);
        w.phaseHopping->setCurrentIndex(plp.mimo_ph);

        updateFieldVisibility(w);
        updateAllocModeUi(w);
    }
}

void PlpPage::saveToConfig(AtscConfig& config) {
    for (int t = 0; t < (int)m_tabMapping.size(); t++) {
        auto [sf, p] = m_tabMapping[t];
        if (sf >= (int)config.subframes.size()) continue;
        auto& sfConfig = config.subframes[sf];
        if (p >= (int)sfConfig.plps.size())
            sfConfig.plps.resize(p + 1);

        auto& plp = sfConfig.plps[p];
        const auto& w = m_widgets[t];

        plp.layer = w.layer->currentData().toInt();
        plp.id = w.id->value();
        plp.lls_flag = w.llsFlag->currentData().toInt();
        plp.alloc_mode = w.allocMode->currentData().toInt();
        plp.start = w.start->value();
        plp.size = w.size->value();
        plp.fec_type = w.fecType->currentData().toInt();
        plp.mod = w.modOrder->currentData().toInt();
        plp.cod = w.codeRate->currentData().toInt();
        plp.fec_block_start = w.fecBlockStart->value();
        plp.ti_mode = w.tiMode->currentData().toInt();
        plp.cti_fec_block_start = w.ctiFecBlockStart->value();
        plp.ti_extended_interleaving = w.tiExtended->currentData().toInt();
        plp.cti_depth = w.ctiDepth->currentData().toInt();
        plp.hti_cell_interleaver = w.cellInterleaver->currentData().toInt();
        plp.hti_inter_subframe = w.interSubframe->currentData().toInt();
        plp.hti_num_ti_blocks = w.numTiBlocks->value();
        plp.hti_num_fec_blocks_max = w.numFecBlocksMax->value();
        plp.hti_num_fec_blocks = w.numFecBlocks->value();
        plp.type = w.plpType->currentData().toInt();
        plp.num_subslices = w.numSubslice->value();
        plp.subslice_interval = w.subsliceInterval->value();
        plp.ldm_injection_level = w.ldmInjectionLevel->currentData().toInt();
        plp.mimo = w.mimoPlp->currentData().toInt();
        plp.mimo_stream_combining = w.streamCombining->currentData().toInt();
        plp.mimo_iq_interleaving = w.iqInterleaving->currentData().toInt();
        plp.mimo_ph = w.phaseHopping->currentData().toInt();
    }
}

void PlpPage::updateAllocModeUi(PlpWidgets& w) {
    bool isManual = (w.allocMode->currentData().toInt() == 1);
    w.start->setReadOnly(!isManual);
    w.size->setReadOnly(!isManual);

    QString style = isManual
        ? ""
        : "QSpinBox { background-color: #1c2128; color: #8b949e; }";
    w.start->setStyleSheet(style);
    w.size->setStyleSheet(style);
}

void PlpPage::recalcAutoPlps(const std::vector<int>& totalCapacities) {
    // Group widgets by subframe
    std::map<int, std::vector<int>> sfToWidgetIndices;
    for (int t = 0; t < (int)m_tabMapping.size(); t++) {
        sfToWidgetIndices[m_tabMapping[t].first].push_back(t);
    }

    for (auto& [sfIdx, widgetIndices] : sfToWidgetIndices) {
        int totalAvailable = (sfIdx < (int)totalCapacities.size()) ? totalCapacities[sfIdx] : 0;

        // Calculate manual usage and find auto PLPs
        int manualUsed = 0;
        std::vector<int> autoIndices;

        for (int t : widgetIndices) {
            auto& w = m_widgets[t];
            bool isManual = (w.allocMode->currentData().toInt() == 1);
            if (isManual) {
                manualUsed += w.size->value();
            } else {
                autoIndices.push_back(t);
            }
        }

        int availableForAuto = std::max(0, totalAvailable - manualUsed);
        int currentStart = 0;

        // Distribute: first auto PLP gets all remaining, others get 0
        for (int t : widgetIndices) {
            auto& w = m_widgets[t];
            bool isManual = (w.allocMode->currentData().toInt() == 1);

            if (!isManual) {
                bool isFirstAuto = (!autoIndices.empty() && autoIndices[0] == t);
                int size = isFirstAuto ? availableForAuto : 0;

                w.start->blockSignals(true);
                w.size->blockSignals(true);
                w.start->setValue(currentStart);
                w.size->setValue(size);
                w.start->blockSignals(false);
                w.size->blockSignals(false);

                currentStart += size;
            } else {
                int size = w.size->value();

                w.start->blockSignals(true);
                w.start->setValue(currentStart);
                w.start->blockSignals(false);

                currentStart += size;
            }
        }
    }
}
