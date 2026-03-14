#pragma once
#include <QObject>
#include <QProgressBar>
#include <memory>
#include "../models/SessionModel.h"
#include "../services/AnalysisThread.h"
#include "../services/ApiClient.h"
#include "../views/MetricsPanel.h"
#include "../views/PitcherMatchPanel.h"
#include "../views/TimelineWidget.h"

// AnalysisPresenter drives the analysis workflow:
//   1. Validates that a video is loaded
//   2. Spawns an AnalysisThread
//   3. Receives the result and writes it to SessionModel
//   4. Updates MetricsPanel, PitcherMatchPanel, TimelineWidget
class AnalysisPresenter : public QObject {
    Q_OBJECT

public:
    explicit AnalysisPresenter(SessionModel*       model,
                                MetricsPanel*       metricsPanel,
                                PitcherMatchPanel*  matchPanel,
                                TimelineWidget*     timeline,
                                ApiClient*          apiClient,
                                QObject* parent = nullptr);

    void startAnalysis(const QString& videoPath);
    bool isRunning() const { return analysisRunning_; }

signals:
    void analysisStarted();
    void analysisProgressChanged(int percent);
    void analysisFinished();
    void analysisError(QString message);

private slots:
    void onAnalysisComplete(AnalysisResult result);
    void onAnalysisError(QString message);
    void onUploadProgress(int percent);

private:
    SessionModel*       model_;
    MetricsPanel*       metricsPanel_;
    PitcherMatchPanel*  matchPanel_;
    TimelineWidget*     timeline_;
    ApiClient*          apiClient_;
    AnalysisThread*     analysisThread_{nullptr};
    bool                analysisRunning_{false};
    QUrl                apiBaseUrl_;
};
