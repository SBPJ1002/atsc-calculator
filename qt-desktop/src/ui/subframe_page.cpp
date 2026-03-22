#include "ui/subframe_page.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>

SubframePage::SubframePage(QWidget* parent) : QWidget(parent) {
    m_tabs = new QTabWidget;
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_tabs);
    setSubframeCount(1);
}

QWidget* SubframePage::createSubframeTab(int index) {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* content = new QWidget;
    auto* form = new QFormLayout(content);
    form->setSpacing(8);
    form->setContentsMargins(16, 16, 16, 16);

    SubframeWidgets w;

    w.mimo = new QComboBox;
    w.mimo->addItem("Disable", 0); w.mimo->addItem("Enable", 1);
    form->addRow("MIMO:", w.mimo);

    w.mimoMixed = new QComboBox;
    w.mimoMixed->addItem("OFF", 0); w.mimoMixed->addItem("ON", 1);
    form->addRow("MIMO Mixed:", w.mimoMixed);

    w.miso = new QComboBox;
    w.miso->addItem("No MISO", 0); w.miso->addItem("64 coefficients", 1);
    w.miso->addItem("256 coefficients", 2); w.miso->addItem("Reserved", 3);
    form->addRow("MISO:", w.miso);

    w.fftSize = new QComboBox;
    w.fftSize->addItem("8K", 0); w.fftSize->addItem("16K", 1); w.fftSize->addItem("32K", 2);
    form->addRow("FFT Size:", w.fftSize);

    w.reducedCarrier = new QComboBox;
    w.reducedCarrier->addItem("0 (97.2%)", 0); w.reducedCarrier->addItem("1 (95.9%)", 1);
    w.reducedCarrier->addItem("2 (94.5%)", 2); w.reducedCarrier->addItem("3 (93.2%)", 3);
    w.reducedCarrier->addItem("4 (91.8%)", 4);
    form->addRow("Reduced Carrier:", w.reducedCarrier);

    w.guardInterval = new QComboBox;
    const int giVals[] = {192,384,512,768,1024,1536,2048,2432,3072,3648,4096,4864};
    for (int i = 0; i < 12; i++)
        w.guardInterval->addItem(QString::number(giVals[i]), i + 1);
    form->addRow("Guard Interval:", w.guardInterval);

    w.numOfdm = new QSpinBox;
    w.numOfdm->setRange(1, 65535);
    w.numOfdm->setValue(50);
    form->addRow("Num OFDM Symbols:", w.numOfdm);

    w.spPattern = new QComboBox;
    const char* spNames[] = {"SP3_2","SP3_4","SP4_2","SP4_4","SP6_2","SP6_4","SP8_2","SP8_4",
                             "SP12_2","SP12_4","SP16_2","SP16_4","SP24_2","SP24_4","SP32_2","SP32_4"};
    for (int i = 0; i < 16; i++) w.spPattern->addItem(spNames[i], i);
    form->addRow("Scattered Pilot Pattern:", w.spPattern);

    w.spBoost = new QComboBox;
    w.spBoost->addItem("0.00 dB", 0); w.spBoost->addItem("2.20 dB", 1);
    w.spBoost->addItem("3.80 dB", 2); w.spBoost->addItem("4.60 dB", 3);
    w.spBoost->addItem("5.30 dB", 4);
    form->addRow("Scattered Pilot Boost:", w.spBoost);

    w.sbsFirst = new QComboBox;
    w.sbsFirst->addItem("OFF", 0); w.sbsFirst->addItem("ON", 1);
    form->addRow("SBS First:", w.sbsFirst);

    w.sbsLast = new QComboBox;
    w.sbsLast->addItem("OFF", 0); w.sbsLast->addItem("ON", 1);
    form->addRow("SBS Last:", w.sbsLast);

    w.freqInterleaver = new QComboBox;
    w.freqInterleaver->addItem("OFF", 0); w.freqInterleaver->addItem("ON", 1);
    form->addRow("Freq. Interleaver:", w.freqInterleaver);

    w.plpCount = new QSpinBox;
    w.plpCount->setRange(1, 64);
    w.plpCount->setValue(1);
    form->addRow("Number of PLPs:", w.plpCount);

    connect(w.plpCount, QOverload<int>::of(&QSpinBox::valueChanged), this, &SubframePage::plpCountChanged);
    connect(w.mimo, QOverload<int>::of(&QComboBox::currentIndexChanged), [w](int idx) {
        w.mimoMixed->setVisible(idx == 0);
    });

    auto emitChanged = [this]() { emit configChanged(); };
    connect(w.mimo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.mimoMixed, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.miso, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.fftSize, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.reducedCarrier, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.guardInterval, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.numOfdm, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(w.spPattern, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.spBoost, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.sbsFirst, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.sbsLast, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.freqInterleaver, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(w.plpCount, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);

    m_widgets.push_back(w);
    scroll->setWidget(content);
    return scroll;
}

void SubframePage::setSubframeCount(int count) {
    while (m_tabs->count() > 0) m_tabs->removeTab(0);
    m_widgets.clear();
    for (int i = 0; i < count; i++)
        m_tabs->addTab(createSubframeTab(i), QString("Subframe %1").arg(i));
}

void SubframePage::loadFromConfig(const AtscConfig& config) {
    if ((int)m_widgets.size() != (int)config.subframes.size())
        setSubframeCount(config.subframes.size());

    for (int i = 0; i < (int)config.subframes.size() && i < (int)m_widgets.size(); i++) {
        const auto& sf = config.subframes[i];
        auto& w = m_widgets[i];
        w.mimo->setCurrentIndex(sf.mimo);
        w.mimoMixed->setCurrentIndex(sf.mimo_mixed);
        w.miso->setCurrentIndex(sf.miso);
        w.fftSize->setCurrentIndex(sf.fft_size);
        w.reducedCarrier->setCurrentIndex(sf.reduced_carriers);
        int giIdx = sf.guard_interval - 1;
        if (giIdx < 0) giIdx = 0;
        if (giIdx > 11) giIdx = 11;
        w.guardInterval->setCurrentIndex(giIdx);
        w.numOfdm->setValue(sf.num_ofdm_symbols);
        w.spPattern->setCurrentIndex(sf.scattered_pilot_pattern);
        w.spBoost->setCurrentIndex(sf.scattered_pilot_boost);
        w.sbsFirst->setCurrentIndex(sf.sbs_first);
        w.sbsLast->setCurrentIndex(sf.sbs_last);
        w.freqInterleaver->setCurrentIndex(sf.frequency_interleaver);
        w.plpCount->setValue(std::max(1, (int)sf.plps.size()));
    }
}

void SubframePage::saveToConfig(AtscConfig& config) {
    config.subframes.resize(m_widgets.size());
    for (int i = 0; i < (int)m_widgets.size(); i++) {
        auto& sf = config.subframes[i];
        const auto& w = m_widgets[i];
        sf.mimo = w.mimo->currentData().toInt();
        sf.mimo_mixed = w.mimoMixed->currentData().toInt();
        sf.miso = w.miso->currentData().toInt();
        sf.fft_size = w.fftSize->currentData().toInt();
        sf.reduced_carriers = w.reducedCarrier->currentData().toInt();
        sf.guard_interval = w.guardInterval->currentData().toInt();
        sf.num_ofdm_symbols = w.numOfdm->value();
        sf.scattered_pilot_pattern = w.spPattern->currentData().toInt();
        sf.scattered_pilot_boost = w.spBoost->currentData().toInt();
        sf.sbs_first = w.sbsFirst->currentData().toInt();
        sf.sbs_last = w.sbsLast->currentData().toInt();
        sf.frequency_interleaver = w.freqInterleaver->currentData().toInt();
        int plpCount = w.plpCount->value();
        if ((int)sf.plps.size() < plpCount)
            sf.plps.resize(plpCount);
    }
}
