#pragma once
#include <QObject>
#include <QProgressBar>
#include <memory>
#include <cmath>
#include "../models/SessionModel.h"
#include "../models/PoseFrame.h"
#include "../models/BiomechanicsMetrics.h"
#include "../services/AnalysisThread.h"
#include "../services/ApiClient.h"
#include "../views/MetricsPanel.h"
#include "../views/TimelineWidget.h"

// AnalysisPresenter drives the analysis workflow:
//   1. Validates that a video is loaded
//   2. Spawns an AnalysisThread
//   3. Receives the result and writes it to SessionModel
//   4. Updates MetricsPanel and TimelineWidget
class AnalysisPresenter : public QObject {
    Q_OBJECT

public:
    explicit AnalysisPresenter(SessionModel*       model,
                                MetricsPanel*       metricsPanel,
                                TimelineWidget*     timeline,
                                ApiClient*          apiClient,
                                QObject* parent = nullptr);

    void startAnalysis(const QString& videoPath);
    bool isRunning() const { return analysisRunning_; }

    // Reusable per-frame metric computation (also used by ComparisonPresenter)
    static BiomechanicsMetrics computeFrameMetrics(const PoseFrame& pose,
                                                    const BiomechanicsMetrics& fallback);

signals:
    void analysisStarted();
    void analysisProgressChanged(int percent);
    void analysisFinished();
    void analysisError(QString message);

private slots:
    void onAnalysisComplete(AnalysisResult result);
    void onAnalysisError(QString message);
    void onUploadProgress(int percent);
    void onFrameChanged(int frameNumber);

private:
    SessionModel*       model_;
    MetricsPanel*       metricsPanel_;
    TimelineWidget*     timeline_;
    ApiClient*          apiClient_;
    AnalysisThread*     analysisThread_{nullptr};
    bool                analysisRunning_{false};
    QUrl                apiBaseUrl_;
};
