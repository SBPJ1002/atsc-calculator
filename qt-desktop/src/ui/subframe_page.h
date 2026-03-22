#ifndef SUBFRAME_PAGE_H
#define SUBFRAME_PAGE_H

#include <QWidget>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include "model/atsc_config.h"

struct SubframeWidgets {
    QComboBox* mimo;
    QComboBox* mimoMixed;
    QComboBox* miso;
    QComboBox* fftSize;
    QComboBox* reducedCarrier;
    QComboBox* guardInterval;
    QSpinBox* numOfdm;
    QComboBox* spPattern;
    QComboBox* spBoost;
    QComboBox* sbsFirst;
    QComboBox* sbsLast;
    QComboBox* freqInterleaver;
    QSpinBox* plpCount;
};

class SubframePage : public QWidget {
    Q_OBJECT
public:
    explicit SubframePage(QWidget* parent = nullptr);
    void setSubframeCount(int count);
    void loadFromConfig(const AtscConfig& config);
    void saveToConfig(AtscConfig& config);

signals:
    void configChanged();
    void plpCountChanged();

private:
    QWidget* createSubframeTab(int index);
    QTabWidget* m_tabs;
    std::vector<SubframeWidgets> m_widgets;
};

#endif
