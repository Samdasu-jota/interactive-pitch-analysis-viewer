#pragma once
#include <QObject>
#include <QString>
#include <memory>
#include "../models/SessionModel.h"
#include "../services/ApiClient.h"
#include "VideoPresenter.h"
#include "AnalysisPresenter.h"

// MainPresenter is the root coordinator.
// It owns VideoPresenter and AnalysisPresenter, and exposes
// the high-level application actions to MainWindow.
class MainPresenter : public QObject {
    Q_OBJECT

public:
    struct Views {
        VideoPlayerWidget* player;
        TimelineWidget*    timeline;
        MetricsPanel*      metricsPanel;
        PitcherMatchPanel* matchPanel;
    };

    explicit MainPresenter(SessionModel* model, ApiClient* apiClient,
                            const Views& views, QObject* parent = nullptr);

    // ── High-level actions ────────────────────────────────────────────────────
    void openVideo();                   // shows QFileDialog
    void loadVideo(const QString& path);
    void startAnalysis();
    void togglePlayback();
    void stepForward();
    void stepBackward();
    void seekToPhase(PitchPhase phase);
    void setLoopEnabled(bool loop);
    void setPoseVisible(bool visible);
    void exportFrame();

    bool isAnalysisRunning() const;

    VideoPresenter*   videoPresenter()    { return videoPresenter_.get(); }
    AnalysisPresenter* analysisPresenter() { return analysisPresenter_.get(); }

signals:
    void statusMessage(QString message);
    void analysisStarted();
    void analysisFinished();
    void analysisProgressChanged(int percent);
    void errorOccurred(QString message);

private:
    SessionModel*      model_;
    ApiClient*         apiClient_;

    std::unique_ptr<VideoPresenter>    videoPresenter_;
    std::unique_ptr<AnalysisPresenter> analysisPresenter_;

    QString            currentVideoPath_;
};
