#ifndef PREAMBLE_PAGE_H
#define PREAMBLE_PAGE_H

#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include "model/atsc_config.h"

class PreamblePage : public QWidget {
    Q_OBJECT
public:
    explicit PreamblePage(QWidget* parent = nullptr);
    void loadFromConfig(const AtscConfig& config);
    void saveToConfig(AtscConfig& config);

signals:
    void configChanged();
    void subframeCountChanged(int count);

private:
    QComboBox* m_l1bVersion;
    QComboBox* m_mimoScatteredPilotEncoding;
    QComboBox* m_detailFecType;
    QComboBox* m_timeInfoFlag;
    QComboBox* m_frameLengthMode;
    QSpinBox* m_frameLength;
    QSpinBox* m_numSubframes;
    QComboBox* m_l1dVersion;
    QSpinBox* m_l1dBsid;
    QWidget* m_frameLengthRow;
    QWidget* m_bsidRow;
};

#endif
