#include "ui/preamble_page.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>

PreamblePage::PreamblePage(QWidget* parent) : QWidget(parent) {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* content = new QWidget;
    auto* form = new QFormLayout(content);
    form->setSpacing(10);
    form->setContentsMargins(20, 20, 20, 20);

    auto* group = new QGroupBox("Preamble Configuration");
    auto* g = new QFormLayout(group);

    m_l1bVersion = new QComboBox;
    for (int i = 0; i < 8; i++) m_l1bVersion->addItem(QString("Version %1").arg(i), i);
    g->addRow("L1B Version:", m_l1bVersion);

    m_mimoScatteredPilotEncoding = new QComboBox;
    m_mimoScatteredPilotEncoding->addItem("0 - Walsh-Hadamard", 0);
    m_mimoScatteredPilotEncoding->addItem("1 - Null Pilot", 1);
    g->addRow("MIMO SP Encoding:", m_mimoScatteredPilotEncoding);

    m_detailFecType = new QComboBox;
    for (int i = 0; i < 7; i++) m_detailFecType->addItem(QString("Mode %1").arg(i), i);
    g->addRow("L1 Detail FEC Type:", m_detailFecType);

    m_timeInfoFlag = new QComboBox;
    m_timeInfoFlag->addItem("0 - Not included", 0);
    m_timeInfoFlag->addItem("1 - ms precision", 1);
    m_timeInfoFlag->addItem("2 - us precision", 2);
    m_timeInfoFlag->addItem("3 - ns precision", 3);
    g->addRow("Time Info Flag:", m_timeInfoFlag);

    m_frameLengthMode = new QComboBox;
    m_frameLengthMode->addItem("0 - Time aligned", 0);
    m_frameLengthMode->addItem("1 - Symbol aligned", 1);
    g->addRow("Frame Length Mode:", m_frameLengthMode);

    m_frameLength = new QSpinBox;
    m_frameLength->setRange(0, 8191);
    m_frameLengthRow = new QWidget;
    auto* flLayout = new QHBoxLayout(m_frameLengthRow);
    flLayout->setContentsMargins(0, 0, 0, 0);
    flLayout->addWidget(m_frameLength);
    g->addRow("Frame Length:", m_frameLengthRow);

    m_numSubframes = new QSpinBox;
    m_numSubframes->setRange(1, 8);
    m_numSubframes->setValue(1);
    g->addRow("Number of Subframes:", m_numSubframes);

    m_l1dVersion = new QComboBox;
    for (int i = 0; i < 16; i++) m_l1dVersion->addItem(QString("Version %1").arg(i), i);
    g->addRow("L1D Version:", m_l1dVersion);

    m_l1dBsid = new QSpinBox;
    m_l1dBsid->setRange(0, 65535);
    m_bsidRow = new QWidget;
    auto* bsLayout = new QHBoxLayout(m_bsidRow);
    bsLayout->setContentsMargins(0, 0, 0, 0);
    bsLayout->addWidget(m_l1dBsid);
    g->addRow("L1D BSID:", m_bsidRow);

    form->addRow(group);
    scroll->setWidget(content);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);

    connect(m_frameLengthMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_frameLengthRow->setVisible(idx == 0);
        emit configChanged();
    });
    m_frameLengthRow->setVisible(false);

    connect(m_l1dVersion, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int idx) {
        m_bsidRow->setVisible(idx > 0);
        emit configChanged();
    });
    m_bsidRow->setVisible(false);

    connect(m_numSubframes, QOverload<int>::of(&QSpinBox::valueChanged), this, &PreamblePage::subframeCountChanged);

    auto emitChanged = [this]() { emit configChanged(); };
    connect(m_l1bVersion, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_mimoScatteredPilotEncoding, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_detailFecType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_timeInfoFlag, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_numSubframes, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
}

void PreamblePage::loadFromConfig(const AtscConfig& config) {
    const auto& p = config.preamble;
    m_l1bVersion->setCurrentIndex(p.l1b_version);
    m_mimoScatteredPilotEncoding->setCurrentIndex(p.l1b_mimo_scattered_pilot_encoding);
    m_detailFecType->setCurrentIndex(p.l1b_detail_fec_type);
    m_timeInfoFlag->setCurrentIndex(p.l1b_time_info_flag);
    m_frameLengthMode->setCurrentIndex(p.l1b_frame_length_mode);
    m_frameLength->setValue(p.l1b_frame_length);
    m_numSubframes->setValue(p.l1b_num_subframes);
    m_l1dVersion->setCurrentIndex(p.l1d_version);
    m_l1dBsid->setValue(p.l1d_bsid);
    m_frameLengthRow->setVisible(p.l1b_frame_length_mode == 0);
    m_bsidRow->setVisible(p.l1d_version > 0);
}

void PreamblePage::saveToConfig(AtscConfig& config) {
    auto& p = config.preamble;
    p.l1b_version = m_l1bVersion->currentData().toInt();
    p.l1b_mimo_scattered_pilot_encoding = m_mimoScatteredPilotEncoding->currentData().toInt();
    p.l1b_detail_fec_type = m_detailFecType->currentData().toInt();
    p.l1b_time_info_flag = m_timeInfoFlag->currentData().toInt();
    p.l1b_frame_length_mode = m_frameLengthMode->currentData().toInt();
    p.l1b_frame_length = m_frameLength->value();
    p.l1b_num_subframes = m_numSubframes->value();
    p.l1d_version = m_l1dVersion->currentData().toInt();
    p.l1d_bsid = m_l1dBsid->value();
}
