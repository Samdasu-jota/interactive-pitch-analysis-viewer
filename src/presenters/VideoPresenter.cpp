#include "VideoPresenter.h"
#include <QDebug>
#include <algorithm>

VideoPresenter::VideoPresenter(SessionModel* model,
                                VideoPlayerWidget* player,
                                TimelineWidget* timeline,
                                QObject* parent)
    : QObject(parent)
    , model_(model)
    , player_(player)
    , timeline_(timeline)
    , playbackTimer_(new QTimer(this)) {
    connect(playbackTimer_, &QTimer::timeout, this, &VideoPresenter::onPlaybackTick);

    // When the timeline scrubs, request that frame
    connect(timeline_, &TimelineWidget::seekRequested,
            this, &VideoPresenter::seekToFrame);
}

VideoPresenter::~VideoPresenter() {
    if (decodeService_) {
        decodeService_->stopService();
        decodeService_->wait();
    }
    if (prefetch_) prefetch_->stop();
}

void VideoPresenter::loadVideo(const QString& path) {
    // Stop previous video if any
    playbackTimer_->stop();
    if (decodeService_) {
        decodeService_->stopService();
        decodeService_->wait();
    }
    if (prefetch_) {
        prefetch_->stop();
        prefetch_.reset();
    }
    if (thumbGen_) thumbGen_->cancel();

    currentFrame_.store(0);
    atEnd_ = false;
    player_->clearFrame();

    // Create decode service (opens VideoCapture in constructor)
    decodeService_ = std::make_unique<VideoDecodeService>(path, this);

    if (!decodeService_->isOpen()) {
        qWarning() << "VideoPresenter: failed to open" << path;
        return;
    }

    // Connect frame delivery — Qt::QueuedConnection is critical here
    connect(decodeService_.get(), &VideoDecodeService::frameReady,
            this, &VideoPresenter::onFrameReady,
            Qt::QueuedConnection);

    connect(decodeService_.get(), &VideoDecodeService::videoMetadataReady,
            this, &VideoPresenter::onVideoMetadata,
            Qt::QueuedConnection);

    // Start the decode thread
    decodeService_->start();

    // Prime with frame 0
    decodeService_->requestFrame(0);

    // Start thumbnail generation
    thumbGen_ = std::make_unique<ThumbnailGenerator>(this);
    connect(thumbGen_.get(), &ThumbnailGenerator::thumbnailReady,
            timeline_, &TimelineWidget::onThumbnailReady,
            Qt::QueuedConnection);

    thumbGen_->generate(path,
                        decodeService_->fps(),
                        decodeService_->totalFrames());

    // Setup prefetch
    auto requestFn = [this](int frame) {
        if (decodeService_) decodeService_->requestFrame(frame);
    };
    prefetch_ = std::make_unique<PrefetchService>(requestFn, 30);
}

void VideoPresenter::onVideoMetadata(int totalFrames, double fps, QSize resolution) {
    totalFrames_ = totalFrames;
    fps_         = fps;

    player_->setVideoSize(resolution.width(), resolution.height());
    timeline_->setTotalFrames(totalFrames, fps);

    model_->setVideoMetadata(
        decodeService_ ? QString() : QString(),
        totalFrames, fps);

    baseIntervalMs_ = static_cast<int>(1000.0 / std::max(fps, 1.0));
    playbackTimer_->setInterval(std::max(16, static_cast<int>(baseIntervalMs_ / speed_)));
}

void VideoPresenter::onFrameReady(QImage frame, int frameNumber) {
    // When paused, discard stale frames that were queued before pause was pressed.
    // Only display frames explicitly requested via seekToFrame() or the last tick.
    if (!playbackTimer_->isActive() && frameNumber != lastRequestedFrame_.load())
        return;

    // Look up pose data for this frame from the session model
    const PoseFrame* pose = model_->poseFrameAt(frameNumber);
    PitchPhase phase = PitchPhase::Unknown;

    if (model_->hasAnalysis()) {
        phase = model_->analysisResult().phases.phaseAtFrame(frameNumber);
    }

    if (pose) {
        player_->setPoseFrame(*pose, phase);
    }
    player_->onFrameReady(std::move(frame), frameNumber);
    timeline_->setCurrentFrame(frameNumber);
    model_->setCurrentFrame(frameNumber);

    // Only advance prefetch while actively playing.
    // Calling setPosition when paused causes a cascade: each prefetched frame
    // triggers another setPosition → 30 more requests → infinite delivery loop.
    if (prefetch_ && playbackTimer_->isActive())
        prefetch_->setPosition(frameNumber, totalFrames_);
}

void VideoPresenter::onPlaybackTick() {
    int next = currentFrame_.fetch_add(1);
    if (next >= totalFrames_ - 1) {
        if (loopEnabled_) {
            next = 0;
            currentFrame_.store(0);
        } else {
            playbackTimer_->stop();
            model_->setPlaying(false);
            atEnd_ = true;
            emit videoEnded();
            return;
        }
    }
    lastRequestedFrame_.store(next);
    if (decodeService_) decodeService_->requestFrame(next);
    emit frameAdvanced(next);
}

void VideoPresenter::play() {
    if (!decodeService_ || !decodeService_->isOpen()) return;
    if (atEnd_) {
        atEnd_ = false;
        currentFrame_.store(0);
        lastRequestedFrame_.store(0);
    }
    if (!playbackTimer_->isActive()) {
        // Stop prefetch during playback: prefetch has no "silent" mode — it
        // emits frameReady for every decoded frame, causing out-of-order
        // display jumps. The timer alone drives frame requests while playing.
        if (prefetch_) {
            prefetch_->stop();
            prefetch_.reset();
        }
        playbackTimer_->start();
        model_->setPlaying(true);
    }
}

void VideoPresenter::pause() {
    playbackTimer_->stop();
    model_->setPlaying(false);
    // Restart prefetch so the cache is warm for fast timeline scrubbing.
    if (!prefetch_ && decodeService_) {
        auto requestFn = [this](int frame) {
            if (decodeService_) decodeService_->requestFrame(frame);
        };
        prefetch_ = std::make_unique<PrefetchService>(requestFn, 30);
        prefetch_->setPosition(currentFrame_.load(), totalFrames_);
    }
}

void VideoPresenter::togglePlayback() {
    if (playbackTimer_->isActive()) pause();
    else                             play();
}

void VideoPresenter::stepForward() {
    pause();
    int next = std::min(currentFrame_.load() + 1, totalFrames_ - 1);
    seekToFrame(next);
}

void VideoPresenter::stepBackward() {
    pause();
    int prev = std::max(currentFrame_.load() - 1, 0);
    seekToFrame(prev);
}

void VideoPresenter::seekToFrame(int frame) {
    frame = std::clamp(frame, 0, totalFrames_ - 1);
    currentFrame_.store(frame);
    lastRequestedFrame_.store(frame);
    if (decodeService_) decodeService_->requestFrame(frame);
}

void VideoPresenter::setPlaybackSpeed(double speed) {
    speed_ = std::clamp(speed, 0.1, 4.0);
    int intervalMs = std::max(16, static_cast<int>(baseIntervalMs_ / speed_));
    playbackTimer_->setInterval(intervalMs);
}

void VideoPresenter::seekToPhase(PitchPhase phase) {
    if (!model_->hasAnalysis()) return;
    const auto& phases = model_->analysisResult().phases;
    int target = -1;

    switch (phase) {
    case PitchPhase::LegLift:       target = phases.legLift;       break;
    case PitchPhase::Stride:        target = phases.footPlant;     break;
    case PitchPhase::ArmCock:       target = phases.maxArmCock;    break;
    case PitchPhase::Release:       target = phases.releasePoint;  break;
    case PitchPhase::FollowThrough: target = phases.followThrough; break;
    default: break;
    }

    if (target >= 0) seekToFrame(target);
}

void VideoPresenter::setLoopEnabled(bool loop) {
    loopEnabled_ = loop;
    model_->setLoopEnabled(loop);
}

bool VideoPresenter::isPlaying() const {
    return playbackTimer_->isActive();
}
