#include "ComparisonPresenter.h"
#include "../utils/JsonParser.h"
#include <QDebug>

ComparisonPresenter::ComparisonPresenter(SessionModel*   model,
                                          ComparisonView* view,
                                          ApiClient*      apiClient,
                                          QObject*        parent)
    : QObject(parent)
    , model_(model)
    , view_(view)
    , apiClient_(apiClient) {
    // When the user seeks, sync the comparison player
    connect(view_, &ComparisonView::seekRequested,
            this, &ComparisonPresenter::onUserSeekRequested);

    connect(apiClient_, &ApiClient::pitcherDetailsReceived,
            this, &ComparisonPresenter::onPitcherDetailsReceived,
            Qt::QueuedConnection);
}

ComparisonPresenter::~ComparisonPresenter() {
    if (comparisonDecode_) {
        comparisonDecode_->stopService();
        comparisonDecode_->wait();
    }
}

void ComparisonPresenter::loadComparisonPitcher(const QString& pitcherId) {
    apiClient_->fetchPitcherDetails(pitcherId);
}

void ComparisonPresenter::onPitcherDetailsReceived(QString pitcherId,
                                                    QByteArray jsonData) {
    PitcherProfile profile = JsonParser::parsePitcherDetails(jsonData);

    if (profile.videoUrl.isEmpty() || !profile.videoUrl.isLocalFile()) {
        emit comparisonError(
            QStringLiteral("Pitcher '%1' has no local video available.").arg(pitcherId));
        return;
    }

    QString videoPath = profile.videoUrl.toLocalFile();

    // Stop previous comparison decode if any
    if (comparisonDecode_) {
        comparisonDecode_->stopService();
        comparisonDecode_->wait();
    }

    comparisonDecode_ = std::make_unique<VideoDecodeService>(videoPath, this);

    if (!comparisonDecode_->isOpen()) {
        emit comparisonError(
            QStringLiteral("Cannot open comparison video: %1").arg(videoPath));
        return;
    }

    connect(comparisonDecode_.get(), &VideoDecodeService::frameReady,
            this, &ComparisonPresenter::onComparisonFrameReady,
            Qt::QueuedConnection);

    comparisonDecode_->start();
    comparisonDecode_->requestFrame(0);

    // Find the match in session model and update comparison view label
    const auto& matches = model_->analysisResult().topMatches;
    for (const auto& m : matches) {
        if (m.profile.id == pitcherId) {
            view_->setComparisonPitcher(m);
            break;
        }
    }

    model_->setComparisonPitcher(pitcherId);
    emit comparisonLoaded(pitcherId);
}

void ComparisonPresenter::onComparisonFrameReady(QImage frame, int frameNumber) {
    view_->comparisonPlayer()->onFrameReady(std::move(frame), frameNumber);
}

void ComparisonPresenter::onUserSeekRequested(int frame) {
    // Sync comparison player to the same frame position
    if (comparisonDecode_ && view_->isSyncEnabled()) {
        comparisonDecode_->requestFrame(frame);
    }
}

void ComparisonPresenter::setSyncEnabled(bool enabled) {
    view_->setSyncEnabled(enabled);
}
