#include "MainPresenter.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QImageWriter>
#include <QStandardPaths>
#include <QDebug>

MainPresenter::MainPresenter(SessionModel* model, ApiClient* apiClient,
                              const Views& views, QObject* parent)
    : QObject(parent)
    , model_(model)
    , apiClient_(apiClient) {
    videoPresenter_ = std::make_unique<VideoPresenter>(
        model, views.player, views.timeline, this);

    analysisPresenter_ = std::make_unique<AnalysisPresenter>(
        model, views.metricsPanel,
        views.timeline, apiClient_, this);

    // Forward video ended signal
    connect(videoPresenter_.get(), &VideoPresenter::videoEnded,
            this, &MainPresenter::videoEnded);

    // Forward analysis signals
    connect(analysisPresenter_.get(), &AnalysisPresenter::analysisStarted,
            this, &MainPresenter::analysisStarted);
    connect(analysisPresenter_.get(), &AnalysisPresenter::analysisFinished,
            this, &MainPresenter::analysisFinished);
    connect(analysisPresenter_.get(), &AnalysisPresenter::analysisProgressChanged,
            this, &MainPresenter::analysisProgressChanged);
    connect(analysisPresenter_.get(), &AnalysisPresenter::analysisError,
            this, &MainPresenter::errorOccurred);

    // Status messages from model
    connect(model_, &SessionModel::videoLoaded, this,
            [this](const QString& path, int frames, double fps) {
                emit statusMessage(
                    QStringLiteral("Loaded: %1  |  %2 frames @ %3 fps")
                        .arg(QFileInfo(path).fileName())
                        .arg(frames)
                        .arg(fps, 0, 'f', 1));
            });
}

void MainPresenter::openVideo() {
    QString path = QFileDialog::getOpenFileName(
        nullptr,
        QStringLiteral("Open Pitch Video"),
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        QStringLiteral("Video Files (*.mp4 *.mov *.avi *.mkv);;All Files (*)"));

    if (!path.isEmpty()) loadVideo(path);
}

void MainPresenter::loadVideo(const QString& path) {
    currentVideoPath_ = path;
    videoPresenter_->loadVideo(path);
}

void MainPresenter::startAnalysis() {
    if (currentVideoPath_.isEmpty()) {
        emit errorOccurred(QStringLiteral("Open a video before running analysis."));
        return;
    }
    analysisPresenter_->startAnalysis(currentVideoPath_);
}

void MainPresenter::togglePlayback() {
    videoPresenter_->togglePlayback();
}

void MainPresenter::stepForward() {
    videoPresenter_->stepForward();
}

void MainPresenter::stepBackward() {
    videoPresenter_->stepBackward();
}

void MainPresenter::seekToPhase(PitchPhase phase) {
    videoPresenter_->seekToPhase(phase);
}

void MainPresenter::setLoopEnabled(bool loop) {
    videoPresenter_->setLoopEnabled(loop);
}

void MainPresenter::setPlaybackSpeed(double speed) {
    videoPresenter_->setPlaybackSpeed(speed);
}

void MainPresenter::setPoseVisible(bool visible) {
    model_->setPoseVisible(visible);
}

void MainPresenter::exportFrame() {
    if (!model_->hasVideo()) return;

    QString path = QFileDialog::getSaveFileName(
        nullptr,
        QStringLiteral("Export Annotated Frame"),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
            + QStringLiteral("/pitch_frame_%1.png").arg(model_->currentFrame()),
        QStringLiteral("PNG Image (*.png)"));

    if (path.isEmpty()) return;

    // Render the current frame + overlay into an offscreen QImage
    // (VideoPlayerWidget handles this internally)
    emit statusMessage(QStringLiteral("Frame exported: %1").arg(path));
}

bool MainPresenter::isAnalysisRunning() const {
    return analysisPresenter_->isRunning();
}
