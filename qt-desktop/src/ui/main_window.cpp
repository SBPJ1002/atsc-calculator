#include "ui/main_window.h"
#include "ui/bootstrap_page.h"
#include "ui/preamble_page.h"
#include "ui/subframe_page.h"
#include "ui/plp_page.h"
#include "ui/results_panel.h"
#include "io/config_file.h"
#include <QMenuBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QStatusBar>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    m_config.ensureSubframes();
    setupUi();
    setupTheme();
    syncConfigToUi();
    recalculate();
}

void MainWindow::setupTheme() {
    qApp->setStyleSheet(R"(
        QMainWindow, QWidget { background-color: #0d1117; color: #e6edf3; }
        QGroupBox { border: 1px solid #30363d; border-radius: 6px; margin-top: 12px; padding-top: 16px; font-weight: bold; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 6px; color: #58a6ff; }
        QComboBox, QSpinBox, QLineEdit {
            background-color: #161b22; border: 1px solid #30363d; border-radius: 4px;
            padding: 4px 8px; color: #e6edf3; min-height: 24px;
        }
        QComboBox:focus, QSpinBox:focus { border-color: #58a6ff; }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background-color: #161b22; color: #e6edf3; selection-background-color: #1f6feb; }
        QPushButton {
            background-color: #21262d; border: 1px solid #30363d; border-radius: 6px;
            padding: 6px 16px; color: #e6edf3; font-weight: bold;
        }
        QPushButton:hover { background-color: #30363d; border-color: #58a6ff; }
        QPushButton:pressed { background-color: #1f6feb; }
        QPushButton[active="true"] { background-color: #1f6feb; border-color: #58a6ff; color: white; }
        QTabWidget::pane { border: 1px solid #30363d; background-color: #0d1117; }
        QTabBar::tab {
            background-color: #161b22; border: 1px solid #30363d; padding: 6px 14px; color: #8b949e;
            border-top-left-radius: 4px; border-top-right-radius: 4px;
        }
        QTabBar::tab:selected { background-color: #0d1117; color: #58a6ff; border-bottom-color: #0d1117; }
        QLabel { color: #e6edf3; }
        QMenuBar { background-color: #161b22; color: #e6edf3; border-bottom: 1px solid #30363d; }
        QMenuBar::item:selected { background-color: #1f6feb; }
        QMenu { background-color: #161b22; color: #e6edf3; border: 1px solid #30363d; }
        QMenu::item:selected { background-color: #1f6feb; }
        QStatusBar { background-color: #161b22; color: #8b949e; border-top: 1px solid #30363d; }
        QScrollArea { border: none; }
        QSplitter::handle { background-color: #30363d; width: 2px; }
    )");
}

void MainWindow::setupUi() {
    setWindowTitle("ATSC 3.0 Signaling Calculator");
    resize(1400, 800);

    auto* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction("Open Config...", QKeySequence::Open, this, &MainWindow::openConfig);
    fileMenu->addAction("Save Config...", QKeySequence::Save, this, &MainWindow::saveConfig);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", QKeySequence::Quit, this, &QMainWindow::close);

    auto* central = new QWidget;
    auto* mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto* stepBar = new QWidget;
    stepBar->setStyleSheet("background-color: #161b22; border-bottom: 1px solid #30363d;");
    auto* stepLayout = new QHBoxLayout(stepBar);
    stepLayout->setContentsMargins(16, 8, 16, 8);
    const char* stepNames[] = {"1. Bootstrap", "2. Preamble", "3. Subframes", "4. PLPs"};
    for (int i = 0; i < 4; i++) {
        m_stepBtns[i] = new QPushButton(stepNames[i]);
        m_stepBtns[i]->setMinimumWidth(140);
        connect(m_stepBtns[i], &QPushButton::clicked, [this, i]() { goToStep(i); });
        stepLayout->addWidget(m_stepBtns[i]);
    }
    stepLayout->addStretch();
    mainLayout->addWidget(stepBar);

    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* leftWidget = new QWidget;
    auto* leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);

    m_stack = new QStackedWidget;
    m_bootstrapPage = new BootstrapPage;
    m_preamblePage = new PreamblePage;
    m_subframePage = new SubframePage;
    m_plpPage = new PlpPage;
    m_stack->addWidget(m_bootstrapPage);
    m_stack->addWidget(m_preamblePage);
    m_stack->addWidget(m_subframePage);
    m_stack->addWidget(m_plpPage);
    leftLayout->addWidget(m_stack, 1);

    auto* bottomBar = new QWidget;
    bottomBar->setStyleSheet("background-color: #161b22; border-top: 1px solid #30363d;");
    auto* bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(16, 8, 16, 8);
    m_prevBtn = new QPushButton("Previous");
    m_nextBtn = new QPushButton("Next");
    auto* calcBtn = new QPushButton("Calculate");
    calcBtn->setStyleSheet("background-color: #238636; border-color: #2ea043;");
    connect(m_prevBtn, &QPushButton::clicked, this, &MainWindow::prevStep);
    connect(m_nextBtn, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(calcBtn, &QPushButton::clicked, this, &MainWindow::recalculate);
    bottomLayout->addWidget(m_prevBtn);
    bottomLayout->addStretch();
    bottomLayout->addWidget(calcBtn);
    bottomLayout->addWidget(m_nextBtn);
    leftLayout->addWidget(bottomBar);

    splitter->addWidget(leftWidget);

    m_resultsPanel = new ResultsPanel;
    splitter->addWidget(m_resultsPanel);
    splitter->setStretchFactor(0, 5);
    splitter->setStretchFactor(1, 4);

    mainLayout->addWidget(splitter, 1);

    setCentralWidget(central);
    statusBar()->showMessage("Ready");

    connect(m_preamblePage, &PreamblePage::subframeCountChanged, [this](int count) {
        m_config.preamble.l1b_num_subframes = count;
        m_config.subframes.resize(count);
        m_config.ensureSubframes();
        m_subframePage->setSubframeCount(count);
        m_plpPage->rebuildTabs(m_config);
    });

    connect(m_subframePage, &SubframePage::plpCountChanged, [this]() {
        syncConfigFromUi();
        m_plpPage->rebuildTabs(m_config);
    });

    connect(m_bootstrapPage, &BootstrapPage::configChanged, this, &MainWindow::recalculate);
    connect(m_preamblePage, &PreamblePage::configChanged, this, &MainWindow::recalculate);
    connect(m_subframePage, &SubframePage::configChanged, this, &MainWindow::recalculate);
    connect(m_plpPage, &PlpPage::configChanged, this, &MainWindow::recalculate);

    updateStepIndicator();
}

void MainWindow::goToStep(int step) {
    if (step < 0 || step > 3) return;
    syncConfigFromUi();
    m_currentStep = step;
    m_stack->setCurrentIndex(step);
    syncConfigToUi();
    updateStepIndicator();
}

void MainWindow::nextStep() { if (m_currentStep < 3) goToStep(m_currentStep + 1); }
void MainWindow::prevStep() { if (m_currentStep > 0) goToStep(m_currentStep - 1); }

void MainWindow::updateStepIndicator() {
    for (int i = 0; i < 4; i++)
        m_stepBtns[i]->setProperty("active", i == m_currentStep);
    m_prevBtn->setEnabled(m_currentStep > 0);
    m_nextBtn->setVisible(m_currentStep < 3);
    for (int i = 0; i < 4; i++) m_stepBtns[i]->style()->unpolish(m_stepBtns[i]);
    for (int i = 0; i < 4; i++) m_stepBtns[i]->style()->polish(m_stepBtns[i]);
}

void MainWindow::syncConfigFromUi() {
    m_bootstrapPage->saveToConfig(m_config);
    m_preamblePage->saveToConfig(m_config);
    m_subframePage->saveToConfig(m_config);
    m_plpPage->saveToConfig(m_config);
}

void MainWindow::syncConfigToUi() {
    m_bootstrapPage->loadFromConfig(m_config);
    m_preamblePage->loadFromConfig(m_config);
    m_subframePage->loadFromConfig(m_config);
    m_plpPage->rebuildTabs(m_config);
}

void MainWindow::recalculate() {
    syncConfigFromUi();

    // Compute results and write back auto-calculated fields
    FrameResult result = m_resultsPanel->computeResults(m_config);
    m_config.preamble.l1b_preamble_num_symbols = std::max(0, result.numPreambleSymbols - 1);
    m_config.preamble.l1b_detail_size_bytes = result.l1dSizeBytes;
    m_config.preamble.l1b_detail_total_cells = result.l1dCells;
    m_config.preamble.l1b_time_offset = result.timeOffset;

    // Auto-allocate PLP sizes
    std::vector<int> totalCapacities;
    for (const auto& sfr : result.subframes)
        totalCapacities.push_back(sfr.totalCapacity);
    m_plpPage->recalcAutoPlps(totalCapacities);

    // Re-sync after auto-allocation and update results
    m_plpPage->saveToConfig(m_config);
    m_resultsPanel->updateResults(m_config);
}

void MainWindow::openConfig() {
    QString path = QFileDialog::getOpenFileName(this, "Open Configuration", "", "Config Files (*.conf);;All Files (*)");
    if (path.isEmpty()) return;
    if (ConfigFile::load(path, m_config)) {
        syncConfigToUi();
        recalculate();
        statusBar()->showMessage("Configuration loaded: " + path, 5000);
    } else {
        QMessageBox::warning(this, "Error", "Failed to load configuration file.");
    }
}

void MainWindow::saveConfig() {
    QString path = QFileDialog::getSaveFileName(this, "Save Configuration", "config.conf", "Config Files (*.conf);;All Files (*)");
    if (path.isEmpty()) return;
    syncConfigFromUi();
    if (ConfigFile::save(path, m_config)) {
        statusBar()->showMessage("Configuration saved: " + path, 5000);
    } else {
        QMessageBox::warning(this, "Error", "Failed to save configuration file.");
    }
}
