#ifndef BOOTSTRAP_PAGE_H
#define BOOTSTRAP_PAGE_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include "model/atsc_config.h"

class BootstrapPage : public QWidget {
    Q_OBJECT
public:
    explicit BootstrapPage(QWidget* parent = nullptr);
    void loadFromConfig(const AtscConfig& config);
    void saveToConfig(AtscConfig& config);

signals:
    void configChanged();

private:
    QComboBox* m_majorVersion;
    QSpinBox* m_minorVersion;
    QComboBox* m_bootstrapSymbol;
    QComboBox* m_eaWakeup;
    QComboBox* m_systemBandwidth;
    QSpinBox* m_bsrCoefficient;
    QSpinBox* m_minTimeToNext;
    QSpinBox* m_preambleStructure;
    QSpinBox* m_numberOfFrames;
};

#endif
