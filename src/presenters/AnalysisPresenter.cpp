#include "AnalysisPresenter.h"
#include <QDebug>

AnalysisPresenter::AnalysisPresenter(SessionModel*      model,
                                      MetricsPanel*      metricsPanel,
                                      PitcherMatchPanel* matchPanel,
                                      TimelineWidget*    timeline,
                                      ApiClient*         apiClient,
                                      QObject*           parent)
    : QObject(parent)
    , model_(model)
    , metricsPanel_(metricsPanel)
    , matchPanel_(matchPanel)
    , timeline_(timeline)
    , apiClient_(apiClient)
    , apiBaseUrl_(apiClient_->baseUrl()) {}

void AnalysisPresenter::startAnalysis(const QString& videoPath) {
    if (analysisRunning_) {
        qWarning() << "AnalysisPresenter: analysis already in progress";
        return;
    }

    if (videoPath.isEmpty()) {
        emit analysisError(QStringLiteral("No video loaded. Open a video first."));
        return;
    }

    analysisRunning_ = true;
    emit analysisStarted();

    // Spawn a one-shot analysis thread.
    // We use Qt::AutoDeleteOnDestruction pattern — connect finished to deleteLater.
    analysisThread_ = new AnalysisThread(videoPath, apiBaseUrl_, this);

    connect(analysisThread_, &AnalysisThread::analysisComplete,
            this, &AnalysisPresenter::onAnalysisComplete,
            Qt::QueuedConnection);

    connect(analysisThread_, &AnalysisThread::analysisError,
            this, &AnalysisPresenter::onAnalysisError,
            Qt::QueuedConnection);

    connect(analysisThread_, &AnalysisThread::uploadProgress,
            this, &AnalysisPresenter::onUploadProgress,
            Qt::QueuedConnection);

    connect(analysisThread_, &AnalysisThread::finished,
            analysisThread_, &QObject::deleteLater);

    analysisThread_->start();
    qDebug() << "AnalysisPresenter: analysis thread started";
}

void AnalysisPresenter::onAnalysisComplete(AnalysisResult result) {
    analysisRunning_ = false;
    analysisThread_  = nullptr;

    qDebug() << "AnalysisPresenter: analysis complete —"
             << result.poseFrames.size() << "frames,"
             << result.topMatches.size() << "matches";

    // Write to session model (UI thread — safe)
    model_->setAnalysisResult(result);

    // Update UI panels
    metricsPanel_->updateMetrics(result.metrics);
    matchPanel_->setMatches(result.topMatches);
    timeline_->setPhaseFrames(result.phases);

    emit analysisFinished();
}

void AnalysisPresenter::onAnalysisError(QString message) {
    analysisRunning_ = false;
    analysisThread_  = nullptr;
    qWarning() << "AnalysisPresenter: error —" << message;
    emit analysisError(message);
}

void AnalysisPresenter::onUploadProgress(int percent) {
    emit analysisProgressChanged(percent);
}
