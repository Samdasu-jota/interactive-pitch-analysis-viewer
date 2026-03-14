#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QByteArray>

// ApiClient wraps QNetworkAccessManager and provides a non-blocking interface
// to the Python FastAPI backend running at localhost:8000.
//
// All methods return immediately. Results are delivered via signals.
// This class lives on the UI thread.
class ApiClient : public QObject {
    Q_OBJECT

public:
    explicit ApiClient(const QUrl& baseUrl, QObject* parent = nullptr);

    // ── Analysis ─────────────────────────────────────────────────────────────

    // Upload a video file for analysis. Multipart POST to /analyze/upload.
    // Emits: analysisResponseReceived, uploadProgressChanged, networkError
    void uploadVideoForAnalysis(const QString& filePath);

    // ── Database ──────────────────────────────────────────────────────────────

    // Fetch the list of all pro pitchers in the backend database.
    // Emits: pitcherListReceived, networkError
    void fetchPitcherList();

    // Fetch details for a single pitcher.
    // Emits: pitcherDetailsReceived, networkError
    void fetchPitcherDetails(const QString& pitcherId);

    // ── Health ────────────────────────────────────────────────────────────────

    // Ping the health endpoint. Emits connectionStatusChanged.
    void checkConnection();

    bool isConnected() const { return connected_; }
    QUrl baseUrl()     const { return baseUrl_; }

signals:
    void analysisResponseReceived(QByteArray jsonData);
    void pitcherListReceived(QByteArray jsonData);
    void pitcherDetailsReceived(QString pitcherId, QByteArray jsonData);
    void uploadProgressChanged(qint64 bytesSent, qint64 bytesTotal);
    void networkError(QString message);
    void connectionStatusChanged(bool connected);

private slots:
    void onAnalysisReplyFinished();
    void onPitcherListReplyFinished();
    void onHealthReplyFinished();

private:
    QNetworkRequest makeRequest(const QString& path) const;

    QNetworkAccessManager nam_;
    QUrl                  baseUrl_;
    bool                  connected_{false};
};
