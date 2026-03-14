#pragma once
#include <QObject>
#include <memory>
#include "../models/SessionModel.h"
#include "../services/VideoDecodeService.h"
#include "../views/ComparisonView.h"
#include "../services/ApiClient.h"

// ComparisonPresenter manages the side-by-side comparison mode.
// It loads the pro pitcher's video (from backend database) into the
// right VideoPlayerWidget and synchronizes playback.
class ComparisonPresenter : public QObject {
    Q_OBJECT

public:
    explicit ComparisonPresenter(SessionModel*   model,
                                  ComparisonView* view,
                                  ApiClient*      apiClient,
                                  QObject*        parent = nullptr);
    ~ComparisonPresenter() override;

    void loadComparisonPitcher(const QString& pitcherId);
    void setSyncEnabled(bool enabled);

signals:
    void comparisonLoaded(const QString& pitcherId);
    void comparisonError(QString message);

private slots:
    void onPitcherDetailsReceived(QString pitcherId, QByteArray jsonData);
    void onComparisonFrameReady(QImage frame, int frameNumber);
    void onUserSeekRequested(int frame);

private:
    SessionModel*   model_;
    ComparisonView* view_;
    ApiClient*      apiClient_;

    std::unique_ptr<VideoDecodeService> comparisonDecode_;
};
