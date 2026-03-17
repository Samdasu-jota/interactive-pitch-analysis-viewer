#include "MainWindow.h"
#include "../views/SpeedControlWidget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDockWidget>
#include <QKeyEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QFile>
#include <QDebug>

MainWindow::MainWindow(const AppConfig& config, QWidget* parent)
    : QMainWindow(parent)
    , config_(config) {
    setWindowTitle(QStringLiteral("Interactive Pitch Analysis Viewer"));
    setMinimumSize(1200, 700);
    resize(1600, 900);
    setAcceptDrops(true);

    // ── Owned singletons ──────────────────────────────────────────────────────
    sessionModel_ = new SessionModel(this);
    apiClient_    = new ApiClient(config_.apiBaseUrl, this);

    // ── Build UI ──────────────────────────────────────────────────────────────
    buildCentralWidget();
    buildDockWidgets();
    buildMenuBar();
    buildToolBar();
    buildStatusBar();

    // ── Presenters ────────────────────────────────────────────────────────────
    MainPresenter::Views views{
        videoPlayer_, timeline_, metricsPanel_
    };
    mainPresenter_ = std::make_unique<MainPresenter>(
        sessionModel_, apiClient_, views, this);

    comparisonPresenter_ = std::make_unique<ComparisonPresenter>(
        sessionModel_, comparisonView_,
        metricsPanel_, comparisonMetricsPanel_,
        apiClient_, this);

    // ── Wire signals ──────────────────────────────────────────────────────────
    connectSignals();

    if (config_.darkTheme) applyDarkTheme();

    startConnectionMonitor();
}

MainWindow::~MainWindow() = default;

// ── Build UI ──────────────────────────────────────────────────────────────────

void MainWindow::buildCentralWidget() {
    centralStack_ = new QStackedWidget(this);
    setCentralWidget(centralStack_);

    // Page 0: single view
    singleViewPage_ = new QWidget(centralStack_);
    auto* layout    = new QVBoxLayout(singleViewPage_);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    videoPlayer_ = new VideoPlayerWidget(singleViewPage_);
    timeline_    = new TimelineWidget(singleViewPage_);

    layout->addWidget(videoPlayer_, 1);
    layout->addWidget(timeline_);

    centralStack_->addWidget(singleViewPage_);

    // Page 1: comparison view
    comparisonView_ = new ComparisonView(centralStack_);
    centralStack_->addWidget(comparisonView_);

    centralStack_->setCurrentIndex(0);
}

void MainWindow::buildDockWidgets() {
    metricsPanel_ = new MetricsPanel(this);
    metricsPanel_->setWindowTitle(QStringLiteral("Biomechanics"));
    metricsPanel_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    addDockWidget(Qt::RightDockWidgetArea, metricsPanel_);

    comparisonMetricsPanel_ = new MetricsPanel(this);
    comparisonMetricsPanel_->setWindowTitle(QStringLiteral("Video B Metrics"));
    comparisonMetricsPanel_->setFeatures(QDockWidget::NoDockWidgetFeatures);
    // Split beside metricsPanel_ once — show/hide used to switch modes
    splitDockWidget(metricsPanel_, comparisonMetricsPanel_, Qt::Horizontal);
    comparisonMetricsPanel_->hide();
}

void MainWindow::buildMenuBar() {
    // File
    QMenu* fileMenu = menuBar()->addMenu(QStringLiteral("&File"));
    fileMenu->addAction(QStringLiteral("Open Video..."),
                        this, [this] { mainPresenter_->openVideo(); },
                        QKeySequence::Open);
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Export Frame..."),
                        this, [this] { mainPresenter_->exportFrame(); },
                        QKeySequence(Qt::CTRL | Qt::Key_E));
    fileMenu->addSeparator();
    fileMenu->addAction(QStringLiteral("Quit"),
                        qApp, &QApplication::quit,
                        QKeySequence::Quit);

    // View
    QMenu* viewMenu = menuBar()->addMenu(QStringLiteral("&View"));
    viewMenu->addAction(QStringLiteral("Single View"),
                        this, &MainWindow::switchToSingleView);
    viewMenu->addAction(QStringLiteral("Comparison View"),
                        this, &MainWindow::switchToComparisonView);
    viewMenu->addSeparator();
    viewMenu->addAction(metricsPanel_->toggleViewAction());

    // Analysis
    QMenu* analysisMenu = menuBar()->addMenu(QStringLiteral("&Analysis"));
    analysisMenu->addAction(QStringLiteral("Analyze Pitch"),
                             this, [this] { mainPresenter_->startAnalysis(); },
                             QKeySequence(Qt::CTRL | Qt::Key_Return));

    // Jump to phase
    QMenu* jumpMenu = analysisMenu->addMenu(QStringLiteral("Jump to Phase"));
    jumpMenu->addAction(QStringLiteral("Leg Lift (1)"), this,
        [this] { mainPresenter_->seekToPhase(PitchPhase::LegLift); },
        QKeySequence(Qt::Key_1));
    jumpMenu->addAction(QStringLiteral("Foot Plant (2)"), this,
        [this] { mainPresenter_->seekToPhase(PitchPhase::Stride); },
        QKeySequence(Qt::Key_2));
    jumpMenu->addAction(QStringLiteral("Release Point (3)"), this,
        [this] { mainPresenter_->seekToPhase(PitchPhase::Release); },
        QKeySequence(Qt::Key_3));
}

void MainWindow::buildToolBar() {
    QToolBar* toolbar = addToolBar(QStringLiteral("Playback"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));

    // Speed control button — opens popup on click
    speedButton_ = new QToolButton(toolbar);
    speedButton_->setText(QStringLiteral("1.00x ▾"));
    speedButton_->setStyleSheet(QStringLiteral("QToolButton { min-width: 58px; padding: 2px 4px; }"));
    connect(speedButton_, &QToolButton::clicked, this, [this] {
        auto* popup = new SpeedControlWidget(
            mainPresenter_->videoPresenter()->currentSpeed(), speedButton_);
        connect(popup, &SpeedControlWidget::speedChanged, this, [this](double s) {
            mainPresenter_->setPlaybackSpeed(s);
            comparisonPresenter_->setPlaybackSpeed(s);
            speedButton_->setText(QStringLiteral("%1x ▾").arg(s, 0, 'f', 2));
        });
        const QPoint pos = speedButton_->mapToGlobal(
            QPoint(0, speedButton_->height() + 2));
        popup->move(pos);
        popup->show();
    });
    toolbar->addWidget(speedButton_);
    toolbar->addSeparator();

    // Play/Pause
    playPauseAction_ = toolbar->addAction(QStringLiteral("▶  Play"),
        this, [this] {
            if (centralStack_->currentIndex() == 1)
                comparisonPresenter_->togglePlayback();
            else
                mainPresenter_->togglePlayback();
        });

    toolbar->addSeparator();

    // Loop
    loopAction_ = toolbar->addAction(QStringLiteral("⟲ Loop"));
    loopAction_->setCheckable(true);
    connect(loopAction_, &QAction::toggled,
            this, [this](bool on) { mainPresenter_->setLoopEnabled(on); });

    // Pose overlay toggle
    poseToggleAction_ = toolbar->addAction(QStringLiteral("🦴 Skeleton"));
    poseToggleAction_->setCheckable(true);
    poseToggleAction_->setChecked(true);
    connect(poseToggleAction_, &QAction::toggled,
            this, [this](bool on) { mainPresenter_->setPoseVisible(on); });

    toolbar->addSeparator();

    // Analyze
    analyzeAction_ = toolbar->addAction(QStringLiteral("⚡ Analyze"),
        this, [this] {
            if (centralStack_->currentIndex() == 1)
                comparisonPresenter_->startAnalysis();
            else
                mainPresenter_->startAnalysis();
        });

    // Compare (toggles between comparison and single view)
    compareAction_ = toolbar->addAction(QStringLiteral("⧉ Compare"),
        this, [this] {
            if (centralStack_->currentIndex() == 1)
                switchToSingleView();
            else
                switchToComparisonView();
        });

    toolbar->addSeparator();

    // Upload progress bar (hidden until analysis starts)
    progressBar_ = new QProgressBar(toolbar);
    progressBar_->setRange(0, 100);
    progressBar_->setFixedWidth(140);
    progressBar_->setFixedHeight(16);
    progressBar_->setVisible(false);
    progressBar_->setStyleSheet(
        QStringLiteral("QProgressBar { border: 1px solid #444; border-radius: 3px; }"
                       "QProgressBar::chunk { background: #0a84ff; border-radius: 2px; }"));
    toolbar->addWidget(progressBar_);
}

void MainWindow::buildStatusBar() {
    statusLabel_ = new QLabel(QStringLiteral("Ready"), this);
    statusLabel_->setMinimumWidth(300);

    frameLabel_ = new QLabel(QStringLiteral("Frame: —"), this);

    connectionLabel_ = new QLabel(QStringLiteral("⬤ Disconnected"), this);
    connectionLabel_->setStyleSheet(QStringLiteral("color: #cc4444;"));

    statusBar()->addWidget(statusLabel_, 1);
    statusBar()->addPermanentWidget(frameLabel_);
    statusBar()->addPermanentWidget(connectionLabel_);
}

// ── Signal wiring ─────────────────────────────────────────────────────────────

void MainWindow::connectSignals() {
    // Presenter → status bar
    connect(mainPresenter_.get(), &MainPresenter::statusMessage,
            statusLabel_, &QLabel::setText);
    connect(mainPresenter_.get(), &MainPresenter::errorOccurred,
            this, [this](const QString& msg) {
                statusLabel_->setText(QStringLiteral("Error: ") + msg);
                progressBar_->setVisible(false);
                analyzeAction_->setEnabled(true);
                QMessageBox::warning(this, QStringLiteral("Analysis Error"), msg);
            });

    // Analysis progress
    connect(mainPresenter_.get(), &MainPresenter::analysisStarted, this, [this] {
        progressBar_->setVisible(true);
        progressBar_->setValue(0);
        analyzeAction_->setEnabled(false);
        statusLabel_->setText(QStringLiteral("Uploading video..."));
    });
    connect(mainPresenter_.get(), &MainPresenter::analysisProgressChanged,
            progressBar_, &QProgressBar::setValue);
    connect(mainPresenter_.get(), &MainPresenter::analysisFinished, this, [this] {
        progressBar_->setVisible(false);
        analyzeAction_->setEnabled(true);
        statusLabel_->setText(QStringLiteral("Analysis complete."));
        metricsPanel_->raise();
    });

    // Playback state → play/pause button label
    connect(sessionModel_, &SessionModel::playbackStateChanged, this, [this](bool playing) {
        playPauseAction_->setText(playing ? QStringLiteral("⏸  Pause")
                                          : QStringLiteral("▶  Play"));
    });

    // Frame counter in status bar
    connect(sessionModel_, &SessionModel::currentFrameChanged, this, [this](int frame) {
        frameLabel_->setText(
            QStringLiteral("Frame: %1 / %2").arg(frame).arg(sessionModel_->totalFrames()));
    });

    // API connection status
    connect(apiClient_, &ApiClient::connectionStatusChanged,
            this, [this](bool connected) {
                if (connected) {
                    connectionLabel_->setText(QStringLiteral("⬤ Connected"));
                    connectionLabel_->setStyleSheet(QStringLiteral("color: #44cc44;"));
                } else {
                    connectionLabel_->setText(QStringLiteral("⬤ Disconnected"));
                    connectionLabel_->setStyleSheet(QStringLiteral("color: #cc4444;"));
                }
            });

    // Click on video → open file if no video loaded, otherwise toggle play/pause
    connect(videoPlayer_, &VideoPlayerWidget::clicked, this, [this] {
        if (!sessionModel_->hasVideo())
            mainPresenter_->openVideo();
        else
            mainPresenter_->togglePlayback();
    });

    // Skeleton toggle → all players
    connect(sessionModel_, &SessionModel::poseVisibilityChanged,
            videoPlayer_, &VideoPlayerWidget::setPoseVisible);
    connect(sessionModel_, &SessionModel::poseVisibilityChanged,
            comparisonView_->userPlayer(), &VideoPlayerWidget::setPoseVisible);
    connect(sessionModel_, &SessionModel::poseVisibilityChanged,
            comparisonView_->comparisonPlayer(), &VideoPlayerWidget::setPoseVisible);

    // ComparisonPresenter playback state → play/pause button label
    connect(comparisonPresenter_.get(), &ComparisonPresenter::playbackStateChanged,
            this, [this](bool playing) {
                if (centralStack_->currentIndex() == 1)
                    playPauseAction_->setText(playing ? QStringLiteral("⏸  Pause")
                                                      : QStringLiteral("▶  Play"));
            });

    // Loop toggle → comparison presenter
    connect(loopAction_, &QAction::toggled,
            this, [this](bool on) { comparisonPresenter_->setLoopEnabled(on); });

    // Video ended → Replay button
    connect(mainPresenter_.get(), &MainPresenter::videoEnded, this, [this] {
        playPauseAction_->setText(QStringLiteral("↺ Replay"));
    });
    connect(comparisonPresenter_.get(), &ComparisonPresenter::videosEnded, this, [this] {
        playPauseAction_->setText(QStringLiteral("↺ Replay"));
    });

    // New video loaded → reset button
    connect(sessionModel_, &SessionModel::videoLoaded,
            this, [this](const QString&, int, double) {
                playPauseAction_->setText(QStringLiteral("▶  Play"));
            });

    // Comparison view: left player click → open Video A
    connect(comparisonView_->userPlayer(), &VideoPlayerWidget::clicked,
            this, [this] {
                QString path = QFileDialog::getOpenFileName(
                    this, QStringLiteral("Open Video A"), {},
                    QStringLiteral("Video Files (*.mp4 *.mov *.avi *.mkv)"));
                if (!path.isEmpty()) comparisonPresenter_->loadVideoA(path);
            });

    // Comparison view: right player click → open Video B
    connect(comparisonView_->comparisonPlayer(), &VideoPlayerWidget::clicked,
            this, [this] {
                QString path = QFileDialog::getOpenFileName(
                    this, QStringLiteral("Open Video B"), {},
                    QStringLiteral("Video Files (*.mp4 *.mov *.avi *.mkv)"));
                if (!path.isEmpty()) comparisonPresenter_->loadVideoB(path);
            });

    // ComparisonPresenter → progress bar + errors
    connect(comparisonPresenter_.get(), &ComparisonPresenter::analysisStarted, this, [this] {
        progressBar_->setVisible(true);
        progressBar_->setValue(0);
        analyzeAction_->setEnabled(false);
        statusLabel_->setText(QStringLiteral("Analyzing videos..."));
    });
    connect(comparisonPresenter_.get(), &ComparisonPresenter::analysisProgressChanged,
            progressBar_, &QProgressBar::setValue);
    connect(comparisonPresenter_.get(), &ComparisonPresenter::analysisFinished, this, [this] {
        progressBar_->setVisible(false);
        analyzeAction_->setEnabled(true);
        statusLabel_->setText(QStringLiteral("Comparison analysis complete."));
    });
    connect(comparisonPresenter_.get(), &ComparisonPresenter::errorOccurred,
            this, [this](const QString& msg) {
                progressBar_->setVisible(false);
                analyzeAction_->setEnabled(true);
                statusLabel_->setText(QStringLiteral("Error: ") + msg);
                QMessageBox::warning(this, QStringLiteral("Analysis Error"), msg);
            });
}

// ── View switching ────────────────────────────────────────────────────────────

void MainWindow::switchToSingleView() {
    comparisonPresenter_->pause();
    comparisonMetricsPanel_->hide();
    metricsPanel_->setWindowTitle(QStringLiteral("Biomechanics"));
    metricsPanel_->raise();
    centralStack_->setCurrentIndex(0);
    compareAction_->setText(QStringLiteral("⧉ Compare"));
}

void MainWindow::switchToComparisonView() {
    metricsPanel_->setWindowTitle(QStringLiteral("Video A Metrics"));
    comparisonMetricsPanel_->show();
    centralStack_->setCurrentIndex(1);
    compareAction_->setText(QStringLiteral("← Back"));
}

// ── Input ─────────────────────────────────────────────────────────────────────

void MainWindow::keyPressEvent(QKeyEvent* event) {
    switch (event->key()) {
    case Qt::Key_Space:
        if (centralStack_->currentIndex() == 1)
            comparisonPresenter_->togglePlayback();
        else
            mainPresenter_->togglePlayback();
        break;
    case Qt::Key_Left:
        mainPresenter_->stepBackward();
        break;
    case Qt::Key_Right:
        mainPresenter_->stepForward();
        break;
    case Qt::Key_1:
        mainPresenter_->seekToPhase(PitchPhase::LegLift);
        break;
    case Qt::Key_2:
        mainPresenter_->seekToPhase(PitchPhase::Stride);
        break;
    case Qt::Key_3:
        mainPresenter_->seekToPhase(PitchPhase::Release);
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty()) {
        QString path = urls.first().toLocalFile();
        if (!path.isEmpty()) mainPresenter_->loadVideo(path);
    }
}

// ── Dark theme ────────────────────────────────────────────────────────────────

void MainWindow::applyDarkTheme() {
    QFile qssFile(QStringLiteral(":/stylesheets/dark_theme.qss"));
    if (qssFile.open(QFile::ReadOnly)) {
        qApp->setStyleSheet(QString::fromUtf8(qssFile.readAll()));
    }
}

// ── Connection monitor ────────────────────────────────────────────────────────

void MainWindow::startConnectionMonitor() {
    connectionTimer_ = new QTimer(this);
    connect(connectionTimer_, &QTimer::timeout,
            apiClient_, &ApiClient::checkConnection);
    connectionTimer_->start(5000);  // ping every 5 seconds
    apiClient_->checkConnection();  // immediate first check
}

void MainWindow::closeEvent(QCloseEvent* event) {
    connectionTimer_->stop();
    QMainWindow::closeEvent(event);
}
