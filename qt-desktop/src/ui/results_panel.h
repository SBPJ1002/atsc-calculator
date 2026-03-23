#ifndef RESULTS_PANEL_H
#define RESULTS_PANEL_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <vector>
#include "model/atsc_config.h"
#include "engine/cnr_tables.h"

struct PlpResult {
    int plpId = 0;
    int bbFrameRate = 0;
    double bitrateMbps = 0.0;
    int numCells = 0;
    int startCell = 0;
    int fecBlockStart = 0;
    int tiCells = 0;
    int fecBlockSize = 0;
    int numFecBlocks = 0;
    CnrTables::CnrResult cnr{};
};

struct SubframeResult {
    double durationMs = 0.0;
    int totalCapacity = 0;
    int dataCells = 0;
    int cellsInDS = 0;
    int cellsInSBS = 0;
    int dummyCells = 0;
    std::vector<PlpResult> plps;
};

struct FrameResult {
    double totalDurationMs = 0.0;
    int l1bCells = 0;
    int cellsInFirst = 0;
    int cellsInNext = 0;
    int l1dCells = 0;
    int l1dSizeBytes = 0;
    int plpCells = 0;
    int numPreambleSymbols = 0;
    int timeOffset = 0;
    std::vector<SubframeResult> subframes;
};

class ResultsPanel : public QWidget {
    Q_OBJECT
public:
    explicit ResultsPanel(QWidget* parent = nullptr);
    void updateResults(const AtscConfig& config);
    FrameResult computeResults(const AtscConfig& config);

private:
    void clearLayout(QLayout* layout);
    QString formatCnr(double v);

    QLabel* m_frameDuration;
    QLabel* m_l1bCells;
    QLabel* m_cellsFirst;
    QLabel* m_cellsNext;
    QLabel* m_l1dCells;
    QLabel* m_l1dBytes;
    QLabel* m_numSymbols;
    QLabel* m_plpCells;
    QLabel* m_timeOffset;

    QVBoxLayout* m_subframesLayout;
    QWidget* m_subframesContainer;
};

#endif
