#include "VideoDecodeService.h"
#include <QDebug>
#include <opencv2/imgproc.hpp>

VideoDecodeService::VideoDecodeService(const QString& videoPath, QObject* parent)
    : QThread(parent)
    , videoPath_(videoPath) {
    // Open capture and read metadata synchronously in the constructor
    // so callers can query totalFrames() before start() is called.
    capture_.open(videoPath_.toStdString());
    if (!capture_.isOpened()) {
        qWarning() << "VideoDecodeService: failed to open" << videoPath_;
        return;
    }

    captureOpen_  = true;
    totalFrames_  = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_COUNT));
    fps_          = capture_.get(cv::CAP_PROP_FPS);
    int w         = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_WIDTH));
    int h         = static_cast<int>(capture_.get(cv::CAP_PROP_FRAME_HEIGHT));
    resolution_   = QSize(w, h);

    if (fps_ <= 0.0) fps_ = 30.0;

    qDebug() << "VideoDecodeService: opened" << videoPath_
             << totalFrames_ << "frames @" << fps_ << "fps"
             << resolution_;
}

VideoDecodeService::~VideoDecodeService() {
    stopService();
    wait();
    if (capture_.isOpened()) capture_.release();
}

void VideoDecodeService::requestFrame(int frameNumber) {
    if (!captureOpen_) return;
    requestQueue_.push(frameNumber);
}

void VideoDecodeService::stopService() {
    running_.store(false);
    requestQueue_.stop();
}

// ── QThread::run() ────────────────────────────────────────────────────────────
// This function executes entirely on the decode thread.

void VideoDecodeService::run() {
    if (!captureOpen_) return;

    // Emit metadata now (we're on the decode thread, but it's the first emission
    // — the UI thread's event loop will receive it via QueuedConnection).
    emit videoMetadataReady(totalFrames_, fps_, resolution_);

    running_.store(true);

    while (running_.load()) {
        int frameNumber = requestQueue_.popLatest();

        if (frameNumber < 0 || !running_.load()) {
            break;  // stopService() was called
        }

        if (frameNumber >= totalFrames_) {
            qWarning() << "VideoDecodeService: requested frame" << frameNumber
                       << "out of range (total:" << totalFrames_ << ")";
            continue;
        }

        QImage decoded = decodeFrame(frameNumber);
        if (!decoded.isNull()) {
            emit frameReady(decoded, frameNumber);
        }
    }

    qDebug() << "VideoDecodeService: decode loop exited";
}

// ── Frame Decoding ────────────────────────────────────────────────────────────

QImage VideoDecodeService::decodeFrame(int frameNumber) {
    // Fast path: return from cache if already decoded
    if (cache_.contains(frameNumber)) {
        return cache_.get(frameNumber);
    }

    // Seek to the requested frame position.
    // cv::CAP_PROP_POS_FRAMES is the frame index (0-based).
    capture_.set(cv::CAP_PROP_POS_FRAMES, static_cast<double>(frameNumber));

    cv::Mat bgrFrame;
    if (!capture_.read(bgrFrame) || bgrFrame.empty()) {
        emit decodeError(QStringLiteral("Failed to decode frame %1").arg(frameNumber));
        return {};
    }

    // Convert BGR (OpenCV default) → RGB (Qt default)
    cv::Mat rgbFrame;
    cv::cvtColor(bgrFrame, rgbFrame, cv::COLOR_BGR2RGB);

    // Wrap in QImage WITHOUT copying (zero-copy on render path).
    // We must deep-copy before storing in the cache because rgbFrame's
    // cv::Mat data will be overwritten on the next capture_.read() call.
    QImage img = matToQImage(rgbFrame);
    QImage cached = img.copy();     // deep copy into persistent buffer

    cache_.insert(frameNumber, cached);
    return cached;
}

// static
QImage VideoDecodeService::matToQImage(const cv::Mat& mat) {
    if (mat.empty()) return {};

    // Assumes mat is already RGB888 (3-channel, 8-bit, no padding issues if
    // step == cols * 3). We pass step to handle any row alignment padding.
    return QImage(
        mat.data,
        mat.cols,
        mat.rows,
        static_cast<int>(mat.step),
        QImage::Format_RGB888
    );
}
