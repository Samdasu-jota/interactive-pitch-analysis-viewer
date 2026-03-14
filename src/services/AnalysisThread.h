#pragma once
#include <QThread>
#include <QUrl>
#include <QByteArray>
#include "../models/AnalysisResult.h"

// AnalysisThread is an on-demand QThread that orchestrates a full analysis:
//   1. Upload the video file via multipart POST (blocking this thread only).
//   2. Wait for the HTTP response.
//   3. Parse the JSON into an AnalysisResult.
//   4. Emit analysisComplete() via QueuedConnection to the UI thread.
//   5. Die (QThread auto-deletes if deleteOnFinish is set).
//
// The UI thread spawns this thread, connects its signals, then forgets about it.
// The analysis may take 30–120 seconds; the UI thread remains completely
// responsive throughout.
class AnalysisThread : public QThread {
    Q_OBJECT

public:
    explicit AnalysisThread(const QString& videoPath, const QUrl& apiBaseUrl,
                             QObject* parent = nullptr);

signals:
    // Emitted on success. Delivered to UI thread via QueuedConnection.
    void analysisComplete(AnalysisResult result);

    // Emitted on failure.
    void analysisError(QString message);

    // Upload progress (0–100).
    void uploadProgress(int percent);

protected:
    void run() override;

private:
    QString videoPath_;
    QUrl    apiBaseUrl_;
};
