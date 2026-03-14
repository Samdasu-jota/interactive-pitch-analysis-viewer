#include "AnalysisThread.h"
#include "../utils/JsonParser.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QEventLoop>
#include <QDebug>

AnalysisThread::AnalysisThread(const QString& videoPath, const QUrl& apiBaseUrl,
                                QObject* parent)
    : QThread(parent)
    , videoPath_(videoPath)
    , apiBaseUrl_(apiBaseUrl) {}

void AnalysisThread::run() {
    qDebug() << "AnalysisThread: starting analysis for" << videoPath_;

    // QNetworkAccessManager must be created on the thread that uses it.
    QNetworkAccessManager nam;

    // ── Build multipart request ───────────────────────────────────────────────
    QFile* file = new QFile(videoPath_);
    if (!file->open(QIODevice::ReadOnly)) {
        emit analysisError(QStringLiteral("Cannot open video file: %1").arg(videoPath_));
        delete file;
        return;
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart filePart;
    QFileInfo info(videoPath_);
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QStringLiteral("form-data; name=\"file\"; filename=\"%1\"")
                           .arg(info.fileName()));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("video/mp4"));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QUrl url = apiBaseUrl_;
    url.setPath(QStringLiteral("/analyze/upload"));
    QNetworkRequest req(url);

    // ── Post and block THIS thread (not UI) until response arrives ────────────
    QNetworkReply* reply = nam.post(req, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::uploadProgress,
            this, [this](qint64 sent, qint64 total) {
                if (total > 0) {
                    emit uploadProgress(static_cast<int>(100.0 * sent / total));
                }
            });

    // Block this thread by running a local event loop until the reply finishes.
    // This does NOT block the UI thread because we are on a separate QThread.
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // ── Process response ──────────────────────────────────────────────────────
    if (reply->error() != QNetworkReply::NoError) {
        int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString msg  = QStringLiteral("Analysis API error: %1 (HTTP %2)")
                           .arg(reply->errorString()).arg(httpCode);
        qWarning() << "AnalysisThread:" << msg;
        emit analysisError(msg);
        reply->deleteLater();
        return;
    }

    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    qDebug() << "AnalysisThread: parsing" << responseData.size() << "bytes of JSON";

    AnalysisResult result = JsonParser::parseAnalysisResponse(responseData);

    if (result.valid) {
        qDebug() << "AnalysisThread: analysis complete,"
                 << result.poseFrames.size() << "pose frames";
        emit analysisComplete(result);  // Qt::QueuedConnection → UI thread
    } else {
        emit analysisError(QStringLiteral("Failed to parse analysis response"));
    }
}
