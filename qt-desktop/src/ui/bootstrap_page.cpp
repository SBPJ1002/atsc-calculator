#include "ui/bootstrap_page.h"
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QVBoxLayout>

BootstrapPage::BootstrapPage(QWidget* parent) : QWidget(parent) {
    auto* scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    auto* content = new QWidget;
    auto* form = new QFormLayout(content);
    form->setSpacing(10);
    form->setContentsMargins(20, 20, 20, 20);

    auto* group = new QGroupBox("Bootstrap Configuration");
    auto* gLayout = new QFormLayout(group);

    m_majorVersion = new QComboBox;
    m_majorVersion->addItem("0 (137)", 137);
    m_majorVersion->addItem("1 (400)", 400);
    gLayout->addRow("Major Version:", m_majorVersion);

    m_minorVersion = new QSpinBox;
    m_minorVersion->setRange(0, 255);
    gLayout->addRow("Minor Version:", m_minorVersion);

    m_bootstrapSymbol = new QComboBox;
    m_bootstrapSymbol->addItem("4", 4);
    m_bootstrapSymbol->addItem("5", 5);
    gLayout->addRow("Bootstrap Symbol:", m_bootstrapSymbol);

    m_eaWakeup = new QComboBox;
    m_eaWakeup->addItem("0 - No information", 0);
    m_eaWakeup->addItem("1", 1);
    m_eaWakeup->addItem("2", 2);
    gLayout->addRow("EA Wakeup:", m_eaWakeup);

    m_systemBandwidth = new QComboBox;
    m_systemBandwidth->addItem("6 MHz", 6);
    m_systemBandwidth->addItem("7 MHz", 7);
    m_systemBandwidth->addItem("8 MHz", 8);
    gLayout->addRow("System Bandwidth:", m_systemBandwidth);

    m_bsrCoefficient = new QSpinBox;
    m_bsrCoefficient->setRange(0, 11);
    gLayout->addRow("BSR Coefficient:", m_bsrCoefficient);

    m_minTimeToNext = new QSpinBox;
    m_minTimeToNext->setRange(0, 8191);
    gLayout->addRow("Min Time to Next:", m_minTimeToNext);

    m_preambleStructure = new QSpinBox;
    m_preambleStructure->setRange(0, 255);
    gLayout->addRow("Preamble Structure:", m_preambleStructure);

    m_numberOfFrames = new QSpinBox;
    m_numberOfFrames->setRange(1, 100);
    m_numberOfFrames->setValue(10);
    gLayout->addRow("Number of Frames:", m_numberOfFrames);

    form->addRow(group);
    scroll->setWidget(content);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);

    auto emitChanged = [this]() { emit configChanged(); };
    connect(m_majorVersion, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_minorVersion, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(m_bootstrapSymbol, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_eaWakeup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_systemBandwidth, QOverload<int>::of(&QComboBox::currentIndexChanged), this, emitChanged);
    connect(m_bsrCoefficient, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(m_minTimeToNext, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(m_preambleStructure, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
    connect(m_numberOfFrames, QOverload<int>::of(&QSpinBox::valueChanged), this, emitChanged);
}

void BootstrapPage::loadFromConfig(const AtscConfig& config) {
    const auto& b = config.bootstrap;
    m_majorVersion->setCurrentIndex(b.major_version == 400 ? 1 : 0);
    m_minorVersion->setValue(b.minor_version);
    m_bootstrapSymbol->setCurrentIndex(b.bootstrap_symbol == 5 ? 1 : 0);
    m_eaWakeup->setCurrentIndex(b.ea_wakeup);
    int bwIdx = b.system_bandwidth - 6;
    if (bwIdx < 0 || bwIdx > 2) bwIdx = 0;
    m_systemBandwidth->setCurrentIndex(bwIdx);
    m_bsrCoefficient->setValue(b.bsr_coefficient);
    m_minTimeToNext->setValue(b.min_time_to_next);
    m_preambleStructure->setValue(b.preamble_structure);
    m_numberOfFrames->setValue(b.number_of_frames);
}

void BootstrapPage::saveToConfig(AtscConfig& config) {
    auto& b = config.bootstrap;
    b.major_version = m_majorVersion->currentData().toInt();
    b.minor_version = m_minorVersion->value();
    b.bootstrap_symbol = m_bootstrapSymbol->currentData().toInt();
    b.ea_wakeup = m_eaWakeup->currentData().toInt();
    b.system_bandwidth = m_systemBandwidth->currentData().toInt();
    b.bsr_coefficient = m_bsrCoefficient->value();
    b.min_time_to_next = m_minTimeToNext->value();
    b.preamble_structure = m_preambleStructure->value();
    b.number_of_frames = m_numberOfFrames->value();
}
