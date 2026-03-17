#pragma once
#include <QObject>
#include <QTimer>
#include <memory>
#include <atomic>
#include <cmath>
#include "../models/SessionModel.h"
#include "../models/AnalysisResult.h"
#include "../models/PoseFrame.h"
#include "../models/BiomechanicsMetrics.h"
#include "../services/VideoDecodeService.h"
#include "../services/AnalysisThread.h"
#include "../services/ApiClient.h"
#include "../views/ComparisonView.h"
#include "../views/MetricsPanel.h"

// ComparisonPresenter manages independent side-by-side comparison:
//   - Video A (left) and Video B (right) are loaded independently via file dialog.
//   - A single playback timer drives both videos simultaneously.
//   - Analyze runs both through the API sequentially.
//   - Each analysis result populates its respective MetricsPanel.
//   - During playback, per-frame metrics and skeleton update both dashboards in real-time.
class ComparisonPresenter : public QObject {
    Q_OBJECT

public:
    explicit ComparisonPresenter(SessionModel*   model,
                                  ComparisonView* view,
                                  MetricsPanel*   leftMetrics,
                                  MetricsPanel*   rightMetrics,
                                  ApiClient*      apiClient,
                                  QObject*        parent = nullptr);
    ~ComparisonPresenter() override;

    void loadVideoA(const QString& path);
    void loadVideoB(const QString& path);
    void startAnalysis();

    // Playback control
    void play();
    void pause();
    void togglePlayback();
    void setPlaybackSpeed(double speed);
    void setLoopEnabled(bool loop) { loopEnabled_ = loop; }
    bool isPlaying() const { return playing_; }
    double currentSpeed() const { return speed_; }

    // Seeking
    void seekA(int frame);
    void seekB(int frame);

signals:
    void analysisStarted();
    void analysisProgressChanged(int percent);
    void analysisFinished();
    void errorOccurred(QString message);
    void playbackStateChanged(bool playing);
    void videosEnded();

private slots:
    void onPlaybackTick();
    void onMetadataAReady(int totalFrames, double fps, QSize resolution);
    void onMetadataBReady(int totalFrames, double fps, QSize resolution);
    void onFrameAReady(QImage frame, int frameNumber);
    void onFrameBReady(QImage frame, int frameNumber);
    void onAnalysisAComplete(AnalysisResult result);
    void onAnalysisBComplete(AnalysisResult result);
    void onAnalysisError(QString message);
    void onUploadProgress(int percent);

private:
    void startAnalysisB();

    SessionModel*   model_;
    ComparisonView* view_;
    MetricsPanel*   leftMetrics_;
    MetricsPanel*   rightMetrics_;
    ApiClient*      apiClient_;
    QUrl            apiBaseUrl_;

    std::unique_ptr<VideoDecodeService> decodeA_;
    std::unique_ptr<VideoDecodeService> decodeB_;

    QString         pathA_;
    QString         pathB_;
    AnalysisResult  resultA_;
    AnalysisResult  resultB_;

    // 0=idle, 1=analyzing A, 2=analyzing B, 3=done
    int             analysisPhase_{0};

    // Playback
    QTimer*              playbackTimer_{nullptr};
    std::atomic<int>     currentFrameA_{0};
    std::atomic<int>     currentFrameB_{0};
    std::atomic<int>     lastRequestedFrameA_{-1};
    std::atomic<int>     lastRequestedFrameB_{-1};
    int                  totalFramesA_{0};
    int                  totalFramesB_{0};
    int                  baseIntervalMs_{33};
    double               speed_{1.0};
    bool                 loopEnabled_{false};
    bool                 playing_{false};
    bool                 atEnd_{false};
    bool                 aEnded_{false};
    bool                 bEnded_{false};
};
