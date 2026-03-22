#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QLabel>
#include "model/atsc_config.h"

class BootstrapPage;
class PreamblePage;
class SubframePage;
class PlpPage;
class ResultsPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void goToStep(int step);
    void nextStep();
    void prevStep();
    void openConfig();
    void saveConfig();
    void recalculate();

private:
    void setupUi();
    void setupTheme();
    void updateStepIndicator();
    void syncConfigFromUi();
    void syncConfigToUi();

    AtscConfig m_config;

    QStackedWidget* m_stack;
    QPushButton* m_stepBtns[4];
    QPushButton* m_prevBtn;
    QPushButton* m_nextBtn;
    int m_currentStep = 0;

    BootstrapPage* m_bootstrapPage;
    PreamblePage* m_preamblePage;
    SubframePage* m_subframePage;
    PlpPage* m_plpPage;
    ResultsPanel* m_resultsPanel;
};

#endif
