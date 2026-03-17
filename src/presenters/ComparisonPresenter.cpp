#include "ComparisonPresenter.h"
#include "AnalysisPresenter.h"
#include <QDebug>
#include <algorithm>

ComparisonPresenter::ComparisonPresenter(SessionModel*   model,
                                          ComparisonView* view,
                                          MetricsPanel*   leftMetrics,
                                          MetricsPanel*   rightMetrics,
                                          ApiClient*      apiClient,
                                          QObject*        parent)
    : QObject(parent)
    , model_(model)
    , view_(view)
    , leftMetrics_(leftMetrics)
    , rightMetrics_(rightMetrics)
    , apiClient_(apiClient)
    , apiBaseUrl_(apiClient_->baseUrl())
{
    playbackTimer_ = new QTimer(this);
    connect(playbackTimer_, &QTimer::timeout,
            this, &ComparisonPresenter::onPlaybackTick);

    // Wire timeline scrubbing
    connect(view_->userTimeline(), &TimelineWidget::seekRequested,
            this, &ComparisonPresenter::seekA);
    connect(view_->compTimeline(), &TimelineWidget::seekRequested,
            this, &ComparisonPresenter::seekB);
}

ComparisonPresenter::~ComparisonPresenter() {
    playbackTimer_->stop();
    if (decodeA_) { decodeA_->stopService(); decodeA_->wait(); }
    if (decodeB_) { decodeB_->stopService(); decodeB_->wait(); }
}

// ── Video loading ─────────────────────────────────────────────────────────────

void ComparisonPresenter::loadVideoA(const QString& path) {
    pause();
    if (decodeA_) { decodeA_->stopService(); decodeA_->wait(); }

    decodeA_ = std::make_unique<VideoDecodeService>(path, this);
    if (!decodeA_->isOpen()) {
        emit errorOccurred(QStringLiteral("Cannot open Video A: %1").arg(path));
        return;
    }

    connect(decodeA_.get(), &VideoDecodeService::videoMetadataReady,
            this, &ComparisonPresenter::onMetadataAReady, Qt::QueuedConnection);
    connect(decodeA_.get(), &VideoDecodeService::frameReady,
            this, &ComparisonPresenter::onFrameAReady, Qt::QueuedConnection);

    pathA_ = path;
    resultA_ = {};
    atEnd_ = false;
    aEnded_ = false;
    currentFrameA_.store(0);
    lastRequestedFrameA_.store(0);

    decodeA_->start();
    decodeA_->requestFrame(0);

    view_->setUserLabel(QStringLiteral("VIDEO A"));
    qDebug() << "ComparisonPresenter: loaded Video A:" << path;
}

void ComparisonPresenter::loadVideoB(const QString& path) {
    pause();
    if (decodeB_) { decodeB_->stopService(); decodeB_->wait(); }

    decodeB_ = std::make_unique<VideoDecodeService>(path, this);
    if (!decodeB_->isOpen()) {
        emit errorOccurred(QStringLiteral("Cannot open Video B: %1").arg(path));
        return;
    }

    connect(decodeB_.get(), &VideoDecodeService::videoMetadataReady,
            this, &ComparisonPresenter::onMetadataBReady, Qt::QueuedConnection);
    connect(decodeB_.get(), &VideoDecodeService::frameReady,
            this, &ComparisonPresenter::onFrameBReady, Qt::QueuedConnection);

    pathB_ = path;
    resultB_ = {};
    atEnd_ = false;
    bEnded_ = false;
    currentFrameB_.store(0);
    lastRequestedFrameB_.store(0);

    decodeB_->start();
    decodeB_->requestFrame(0);

    qDebug() << "ComparisonPresenter: loaded Video B:" << path;
}

// ── Metadata ──────────────────────────────────────────────────────────────────

void ComparisonPresenter::onMetadataAReady(int totalFrames, double fps, QSize resolution) {
    totalFramesA_ = totalFrames;
    double safeFps = (fps > 1.0) ? fps : 30.0;
    baseIntervalMs_ = std::max(16, static_cast<int>(1000.0 / safeFps));
    view_->userPlayer()->setVideoSize(resolution.width(), resolution.height());
    view_->userTimeline()->setTotalFrames(totalFrames, safeFps);
    qDebug() << "ComparisonPresenter: Video A metadata — frames:" << totalFrames << "fps:" << fps;
}

void ComparisonPresenter::onMetadataBReady(int totalFrames, double fps, QSize resolution) {
    totalFramesB_ = totalFrames;
    double safeFps = (fps > 1.0) ? fps : 30.0;
    view_->comparisonPlayer()->setVideoSize(resolution.width(), resolution.height());
    view_->compTimeline()->setTotalFrames(totalFrames, safeFps);
    qDebug() << "ComparisonPresenter: Video B metadata — frames:" << totalFrames;
}

// ── Playback ──────────────────────────────────────────────────────────────────

void ComparisonPresenter::play() {
    if (!decodeA_ && !decodeB_) return;
    if (atEnd_) {
        atEnd_ = false;
        aEnded_ = false;
        bEnded_ = false;
        currentFrameA_.store(0);
        currentFrameB_.store(0);
    }
    playing_ = true;
    int interval = std::max(16, static_cast<int>(baseIntervalMs_ / speed_));
    playbackTimer_->start(interval);
    emit playbackStateChanged(true);
}

void ComparisonPresenter::pause() {
    playbackTimer_->stop();
    playing_ = false;
    emit playbackStateChanged(false);
}

void ComparisonPresenter::togglePlayback() {
    playing_ ? pause() : play();
}

void ComparisonPresenter::setPlaybackSpeed(double speed) {
    speed_ = std::clamp(speed, 0.1, 4.0);
    if (playing_) {
        int interval = std::max(16, static_cast<int>(baseIntervalMs_ / speed_));
        playbackTimer_->setInterval(interval);
    }
}

void ComparisonPresenter::onPlaybackTick() {
    int nextA = currentFrameA_.fetch_add(1);
    int nextB = currentFrameB_.fetch_add(1);

    bool endA = (totalFramesA_ > 0 && nextA >= totalFramesA_);
    bool endB = (totalFramesB_ > 0 && nextB >= totalFramesB_);

    // Handle each video independently
    if (endA) {
        if (loopEnabled_) { currentFrameA_.store(0); nextA = 0; aEnded_ = false; }
        else { currentFrameA_.store(std::max(0, totalFramesA_ - 1)); aEnded_ = true; }
    }
    if (endB) {
        if (loopEnabled_) { currentFrameB_.store(0); nextB = 0; bEnded_ = false; }
        else { currentFrameB_.store(std::max(0, totalFramesB_ - 1)); bEnded_ = true; }
    }

    // Pause only when BOTH videos have ended (use persistent flags to avoid oscillation)
    if (!loopEnabled_ && aEnded_ && bEnded_) {
        atEnd_ = true;
        pause();            // emits playbackStateChanged(false) → "▶ Play"
        emit videosEnded(); // overrides → "↺ Replay"
        return;
    }

    // Only advance videos that are still playing (or looping)
    if (!aEnded_ || loopEnabled_) {
        lastRequestedFrameA_.store(nextA);
        if (decodeA_) decodeA_->requestFrame(nextA);
    }
    if (!bEnded_ || loopEnabled_) {
        lastRequestedFrameB_.store(nextB);
        if (decodeB_) decodeB_->requestFrame(nextB);
    }
}

// ── Analysis ──────────────────────────────────────────────────────────────────

void ComparisonPresenter::startAnalysis() {
    if (pathA_.isEmpty() || pathB_.isEmpty()) {
        emit errorOccurred(QStringLiteral(
            "Load both Video A and Video B before running analysis."));
        return;
    }

    analysisPhase_ = 1;
    emit analysisStarted();
    qDebug() << "ComparisonPresenter: analyzing Video A...";

    auto* threadA = new AnalysisThread(pathA_, apiBaseUrl_, this);
    connect(threadA, &AnalysisThread::analysisComplete,
            this, &ComparisonPresenter::onAnalysisAComplete, Qt::QueuedConnection);
    connect(threadA, &AnalysisThread::analysisError,
            this, &ComparisonPresenter::onAnalysisError,     Qt::QueuedConnection);
    connect(threadA, &AnalysisThread::uploadProgress,
            this, &ComparisonPresenter::onUploadProgress,    Qt::QueuedConnection);
    connect(threadA, &AnalysisThread::finished, threadA, &QObject::deleteLater);
    threadA->start();
}

void ComparisonPresenter::startAnalysisB() {
    qDebug() << "ComparisonPresenter: analyzing Video B...";
    emit analysisProgressChanged(0);

    auto* threadB = new AnalysisThread(pathB_, apiBaseUrl_, this);
    connect(threadB, &AnalysisThread::analysisComplete,
            this, &ComparisonPresenter::onAnalysisBComplete, Qt::QueuedConnection);
    connect(threadB, &AnalysisThread::analysisError,
            this, &ComparisonPresenter::onAnalysisError,     Qt::QueuedConnection);
    connect(threadB, &AnalysisThread::uploadProgress,
            this, &ComparisonPresenter::onUploadProgress,    Qt::QueuedConnection);
    connect(threadB, &AnalysisThread::finished, threadB, &QObject::deleteLater);
    threadB->start();
}

void ComparisonPresenter::onAnalysisAComplete(AnalysisResult result) {
    resultA_ = std::move(result);
    leftMetrics_->updateMetrics(resultA_.metrics);
    view_->userTimeline()->setPhaseFrames(resultA_.phases);
    qDebug() << "ComparisonPresenter: Video A analysis done.";

    analysisPhase_ = 2;
    startAnalysisB();
}

void ComparisonPresenter::onAnalysisBComplete(AnalysisResult result) {
    resultB_ = std::move(result);
    rightMetrics_->updateMetrics(resultB_.metrics);
    view_->compTimeline()->setPhaseFrames(resultB_.phases);
    qDebug() << "ComparisonPresenter: Video B analysis done.";

    analysisPhase_ = 3;
    emit analysisFinished();
}

void ComparisonPresenter::onAnalysisError(QString message) {
    analysisPhase_ = 0;
    emit errorOccurred(message);
}

void ComparisonPresenter::onUploadProgress(int percent) {
    // Scale: A is 0–50%, B is 50–100%
    int scaled = (analysisPhase_ == 1) ? percent / 2 : 50 + percent / 2;
    emit analysisProgressChanged(scaled);
}

// ── Frame delivery ────────────────────────────────────────────────────────────

void ComparisonPresenter::onFrameAReady(QImage frame, int frameNumber) {
    // Discard stale queued frames when paused
    if (!playing_ && frameNumber != lastRequestedFrameA_.load()) return;

    auto* player = view_->userPlayer();
    player->onFrameReady(frame, frameNumber);
    view_->userTimeline()->setCurrentFrame(frameNumber);

    // Skeleton overlay + real-time metrics
    if (resultA_.valid) {
        const PoseFrame* pose = resultA_.poseAt(frameNumber);
        if (pose && pose->isValid()) {
            player->setPoseFrame(*pose);
            if (analysisPhase_ == 3) {
                leftMetrics_->updateMetrics(
                    AnalysisPresenter::computeFrameMetrics(*pose, resultA_.metrics));
            }
        }
    }
}

void ComparisonPresenter::onFrameBReady(QImage frame, int frameNumber) {
    // Discard stale queued frames when paused
    if (!playing_ && frameNumber != lastRequestedFrameB_.load()) return;

    auto* player = view_->comparisonPlayer();
    player->onFrameReady(frame, frameNumber);
    view_->compTimeline()->setCurrentFrame(frameNumber);

    // Skeleton overlay + real-time metrics
    if (resultB_.valid) {
        const PoseFrame* pose = resultB_.poseAt(frameNumber);
        if (pose && pose->isValid()) {
            player->setPoseFrame(*pose);
            if (analysisPhase_ == 3) {
                rightMetrics_->updateMetrics(
                    AnalysisPresenter::computeFrameMetrics(*pose, resultB_.metrics));
            }
        }
    }
}

// ── Seeking ───────────────────────────────────────────────────────────────────

void ComparisonPresenter::seekA(int frame) {
    if (totalFramesA_ > 0)
        frame = std::clamp(frame, 0, totalFramesA_ - 1);
    currentFrameA_.store(frame);
    lastRequestedFrameA_.store(frame);
    if (decodeA_) decodeA_->requestFrame(frame);
}

void ComparisonPresenter::seekB(int frame) {
    if (totalFramesB_ > 0)
        frame = std::clamp(frame, 0, totalFramesB_ - 1);
    currentFrameB_.store(frame);
    lastRequestedFrameB_.store(frame);
    if (decodeB_) decodeB_->requestFrame(frame);
}
