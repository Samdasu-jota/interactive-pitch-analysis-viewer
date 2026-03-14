#pragma once
#include <QObject>
#include <QString>
#include <atomic>
#include "AnalysisResult.h"

// SessionModel is the single source of truth for application state.
//
// Ownership rules:
//   - Reads are allowed from any thread after signals have been emitted.
//   - Writes MUST happen on the UI thread (or via Qt::QueuedConnection).
//   - State-changing slots emit corresponding change signals.
class SessionModel : public QObject {
    Q_OBJECT

public:
    explicit SessionModel(QObject* parent = nullptr);

    // ── Video metadata ──────────────────────────────────────────────────────
    QString videoPath()     const { return videoPath_; }
    int     totalFrames()   const { return totalFrames_; }
    double  fps()           const { return fps_; }
    bool    hasVideo()      const { return totalFrames_ > 0; }

    // ── Analysis ────────────────────────────────────────────────────────────
    bool                   hasAnalysis()    const { return analysisResult_.valid; }
    const AnalysisResult&  analysisResult() const { return analysisResult_; }
    const PoseFrame*       poseFrameAt(int frameNumber) const;

    // ── Playback state ───────────────────────────────────────────────────────
    int  currentFrame()    const { return currentFrame_.load(); }
    bool isPlaying()       const { return playing_.load(); }
    bool isLoopEnabled()   const { return loopEnabled_; }
    bool isPoseVisible()   const { return poseVisible_; }

    // ── Comparison ───────────────────────────────────────────────────────────
    bool             hasComparisonPitcher() const { return !comparisonPitcherId_.isEmpty(); }
    QString          comparisonPitcherId()  const { return comparisonPitcherId_; }

public slots:
    void setVideoMetadata(const QString& path, int totalFrames, double fps);
    void setAnalysisResult(const AnalysisResult& result);
    void setCurrentFrame(int frameNumber);
    void setPlaying(bool playing);
    void setLoopEnabled(bool enabled);
    void setPoseVisible(bool visible);
    void setComparisonPitcher(const QString& pitcherId);
    void clearAnalysis();

signals:
    void videoLoaded(const QString& path, int totalFrames, double fps);
    void analysisResultReady(const AnalysisResult& result);
    void currentFrameChanged(int frameNumber);
    void playbackStateChanged(bool playing);
    void poseVisibilityChanged(bool visible);
    void comparisonPitcherChanged(const QString& pitcherId);
    void analysisCleared();

private:
    QString         videoPath_;
    int             totalFrames_{0};
    double          fps_{30.0};

    AnalysisResult  analysisResult_;

    std::atomic<int>  currentFrame_{0};
    std::atomic<bool> playing_{false};
    bool              loopEnabled_{false};
    bool              poseVisible_{true};
    QString           comparisonPitcherId_;
};
