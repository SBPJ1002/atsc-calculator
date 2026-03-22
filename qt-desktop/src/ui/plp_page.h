#ifndef PLP_PAGE_H
#define PLP_PAGE_H

#include <QWidget>
#include <QTabWidget>
#include <QComboBox>
#include <QSpinBox>
#include "model/atsc_config.h"

struct PlpWidgets {
    QComboBox* layer;
    QSpinBox* id;
    QComboBox* llsFlag;
    QSpinBox* start;
    QSpinBox* size;
    QComboBox* fecType;
    QComboBox* modOrder;
    QComboBox* codeRate;
    QComboBox* tiMode;
    QComboBox* tiExtended;
    QComboBox* ctiDepth;
    QComboBox* cellInterleaver;
    QComboBox* interSubframe;
    QSpinBox* numTiBlocks;
    QSpinBox* numFecBlocksMax;
    QSpinBox* numFecBlocks;
    QComboBox* plpType;
    QSpinBox* numSubslice;
    QSpinBox* subsliceInterval;
    QComboBox* ldmInjectionLevel;
    QComboBox* mimoPlp;
    QComboBox* streamCombining;
    QComboBox* iqInterleaving;
    QComboBox* phaseHopping;

    QWidget* ctiDepthRow;
    QWidget* cellInterleaverRow;
    QWidget* interSubframeRow;
    QWidget* numTiBlocksRow;
    QWidget* numFecBlocksMaxRow;
    QWidget* numFecBlocksRow;
    QWidget* plpTypeRow;
    QWidget* numSubsliceRow;
    QWidget* subsliceIntervalRow;
    QWidget* ldmInjectionRow;
    QWidget* tiExtendedRow;
    QWidget* mimoPlpRow;
    QWidget* streamCombiningRow;
    QWidget* iqInterleavingRow;
    QWidget* phaseHoppingRow;
};

class PlpPage : public QWidget {
    Q_OBJECT
public:
    explicit PlpPage(QWidget* parent = nullptr);
    void rebuildTabs(const AtscConfig& config);
    void loadFromConfig(const AtscConfig& config);
    void saveToConfig(AtscConfig& config);

signals:
    void configChanged();

private:
    QWidget* createPlpTab(int sfIdx, int plpIdx);
    void updateFieldVisibility(PlpWidgets& w);

    QTabWidget* m_tabs;
    std::vector<PlpWidgets> m_widgets;

    std::vector<std::pair<int,int>> m_tabMapping;
};

#endif
