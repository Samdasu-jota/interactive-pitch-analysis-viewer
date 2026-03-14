#include "ApiClient.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>

ApiClient::ApiClient(const QUrl& baseUrl, QObject* parent)
    : QObject(parent)
    , baseUrl_(baseUrl) {}

QNetworkRequest ApiClient::makeRequest(const QString& path) const {
    QUrl url = baseUrl_;
    url.setPath(path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    return req;
}

// ── Analysis ──────────────────────────────────────────────────────────────────

void ApiClient::uploadVideoForAnalysis(const QString& filePath) {
    QFile* file = new QFile(filePath, this);
    if (!file->open(QIODevice::ReadOnly)) {
        emit networkError(QStringLiteral("Cannot open file: %1").arg(filePath));
        file->deleteLater();
        return;
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    QHttpPart filePart;
    QFileInfo info(filePath);
    QString disposition = QStringLiteral("form-data; name=\"file\"; filename=\"%1\"")
                              .arg(info.fileName());
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, disposition);
    filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                       QStringLiteral("video/mp4"));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);  // multiPart takes ownership

    multiPart->append(filePart);

    QUrl uploadUrl = baseUrl_;
    uploadUrl.setPath(QStringLiteral("/analyze/upload"));
    QNetworkRequest req(uploadUrl);

    QNetworkReply* reply = nam_.post(req, multiPart);
    multiPart->setParent(reply);  // reply takes ownership, cleans up on finish

    connect(reply, &QNetworkReply::uploadProgress,
            this, [this](qint64 sent, qint64 total) {
                emit uploadProgressChanged(sent, total);
            });

    connect(reply, &QNetworkReply::finished,
            this, &ApiClient::onAnalysisReplyFinished);

    qDebug() << "ApiClient: uploading" << filePath << "to" << uploadUrl.toString();
}

void ApiClient::onAnalysisReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        qDebug() << "ApiClient: analysis response received, size:" << data.size();
        emit analysisResponseReceived(data);
    } else {
        QString msg = QStringLiteral("Upload failed: %1 (HTTP %2)")
            .arg(reply->errorString())
            .arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        qWarning() << "ApiClient:" << msg;
        emit networkError(msg);
    }

    reply->deleteLater();
}

// ── Database ──────────────────────────────────────────────────────────────────

void ApiClient::fetchPitcherList() {
    QNetworkReply* reply = nam_.get(makeRequest(QStringLiteral("/database/pitchers")));

    connect(reply, &QNetworkReply::finished,
            this, &ApiClient::onPitcherListReplyFinished);
}

void ApiClient::onPitcherListReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() == QNetworkReply::NoError) {
        emit pitcherListReceived(reply->readAll());
    } else {
        emit networkError(QStringLiteral("Pitcher list fetch failed: %1")
                              .arg(reply->errorString()));
    }

    reply->deleteLater();
}

void ApiClient::fetchPitcherDetails(const QString& pitcherId) {
    QString path = QStringLiteral("/database/pitcher/%1").arg(pitcherId);
    QNetworkReply* reply = nam_.get(makeRequest(path));

    connect(reply, &QNetworkReply::finished, this, [this, reply, pitcherId]() {
        if (reply->error() == QNetworkReply::NoError) {
            emit pitcherDetailsReceived(pitcherId, reply->readAll());
        } else {
            emit networkError(
                QStringLiteral("Pitcher details fetch failed: %1").arg(reply->errorString()));
        }
        reply->deleteLater();
    });
}

// ── Health ────────────────────────────────────────────────────────────────────

void ApiClient::checkConnection() {
    QNetworkReply* reply = nam_.get(makeRequest(QStringLiteral("/health")));
    connect(reply, &QNetworkReply::finished,
            this, &ApiClient::onHealthReplyFinished);
}

void ApiClient::onHealthReplyFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    bool ok = (reply->error() == QNetworkReply::NoError);
    if (ok != connected_) {
        connected_ = ok;
        emit connectionStatusChanged(connected_);
    }

    reply->deleteLater();
}
