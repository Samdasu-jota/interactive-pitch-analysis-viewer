#pragma once
#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QProgressBar>
#include <QToolBar>
#include <QTimer>
#include <memory>
#include "../models/SessionModel.h"
#include "../services/ApiClient.h"
#include "../views/VideoPlayerWidget.h"
#include "../views/TimelineWidget.h"
#include "../views/MetricsPanel.h"
#include "../views/PitcherMatchPanel.h"
#include "../views/ComparisonView.h"
#include "../presenters/MainPresenter.h"
#include "../presenters/ComparisonPresenter.h"
#include "AppConfig.h"

// MainWindow is the QMainWindow shell.
// It constructs all views, wires them to presenters, and handles UI-level
// concerns: menu bar, toolbar, status bar, keyboard shortcuts.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const AppConfig& config, QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private:
    void buildMenuBar();
    void buildToolBar();
    void buildCentralWidget();
    void buildDockWidgets();
    void buildStatusBar();
    void connectSignals();
    void applyDarkTheme();
    void startConnectionMonitor();

    void switchToSingleView();
    void switchToComparisonView();
    void onCompareRequested(const QString& pitcherId);

    // ── Owned objects ─────────────────────────────────────────────────────────
    AppConfig           config_;
    SessionModel*       sessionModel_{nullptr};
    ApiClient*          apiClient_{nullptr};

    // ── Central widget ────────────────────────────────────────────────────────
    QStackedWidget*    centralStack_{nullptr};

    // Single-view page
    QWidget*           singleViewPage_{nullptr};
    VideoPlayerWidget* videoPlayer_{nullptr};
    TimelineWidget*    timeline_{nullptr};

    // Comparison page
    ComparisonView*    comparisonView_{nullptr};

    // ── Dock widgets ──────────────────────────────────────────────────────────
    MetricsPanel*       metricsPanel_{nullptr};
    PitcherMatchPanel*  matchPanel_{nullptr};

    // ── Toolbar controls ──────────────────────────────────────────────────────
    QAction*    playPauseAction_{nullptr};
    QAction*    analyzeAction_{nullptr};
    QAction*    compareAction_{nullptr};
    QAction*    poseToggleAction_{nullptr};
    QAction*    loopAction_{nullptr};
    QProgressBar* progressBar_{nullptr};

    // ── Status bar ────────────────────────────────────────────────────────────
    QLabel*     statusLabel_{nullptr};
    QLabel*     connectionLabel_{nullptr};
    QLabel*     frameLabel_{nullptr};

    // ── Presenters ────────────────────────────────────────────────────────────
    std::unique_ptr<MainPresenter>       mainPresenter_;
    std::unique_ptr<ComparisonPresenter> comparisonPresenter_;

    // ── Connection monitor ────────────────────────────────────────────────────
    QTimer*     connectionTimer_{nullptr};
};
