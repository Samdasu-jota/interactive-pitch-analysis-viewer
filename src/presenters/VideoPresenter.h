#pragma once
#include <QObject>
#include <QTimer>
#include <atomic>
#include <memory>
#include "../models/SessionModel.h"
#include "../services/VideoDecodeService.h"
#include "../services/ThumbnailGenerator.h"
#include "../services/PrefetchService.h"
#include "../views/VideoPlayerWidget.h"
#include "../views/TimelineWidget.h"

// VideoPresenter owns the VideoDecodeService and drives playback.
// It wires the timeline and playback controls to the decode service.
//
// Responsibilities:
//   - Load a video and emit metadata to SessionModel
//   - Drive the playback timer (setInterval based on fps)
//   - Handle seek requests from TimelineWidget
//   - Deliver decoded frames to VideoPlayerWidget with pose overlay data
//   - Manage ThumbnailGenerator lifecycle
class VideoPresenter : public QObject {
    Q_OBJECT

public:
    explicit VideoPresenter(SessionModel* model,
                             VideoPlayerWidget* player,
                             TimelineWidget* timeline,
                             QObject* parent = nullptr);
    ~VideoPresenter() override;

    void loadVideo(const QString& path);
    void play();
    void pause();
    void togglePlayback();
    void stepForward();
    void stepBackward();
    void seekToFrame(int frame);
    void seekToPhase(PitchPhase phase);
    void setLoopEnabled(bool loop);

    bool isPlaying() const;
    void setPlaybackSpeed(double speed);
    double currentSpeed() const { return speed_; }

signals:
    void frameAdvanced(int frameNumber);
    void videoEnded();

private slots:
    void onFrameReady(QImage frame, int frameNumber);
    void onVideoMetadata(int totalFrames, double fps, QSize resolution);
    void onPlaybackTick();

private:
    SessionModel*       model_;
    VideoPlayerWidget*  player_;
    TimelineWidget*     timeline_;

    std::unique_ptr<VideoDecodeService>  decodeService_;
    std::unique_ptr<ThumbnailGenerator>  thumbGen_;
    std::unique_ptr<PrefetchService>     prefetch_;

    QTimer*             playbackTimer_;
    std::atomic<int>    currentFrame_{0};
    std::atomic<int>    lastRequestedFrame_{-1};
    int                 totalFrames_{0};
    int                 baseIntervalMs_{33};
    double              fps_{30.0};
    double              speed_{1.0};
    bool                loopEnabled_{false};
    bool                atEnd_{false};
};
