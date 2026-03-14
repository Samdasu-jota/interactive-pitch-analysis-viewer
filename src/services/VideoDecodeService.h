#pragma once
#include <QThread>
#include <QImage>
#include <QSize>
#include <QString>
#include <atomic>
#include <opencv2/videoio.hpp>
#include "../models/FrameCache.h"
#include "../utils/FrameRequestQueue.h"

// VideoDecodeService runs on a dedicated QThread and is the sole owner of the
// cv::VideoCapture instance. OpenCV's VideoCapture is not thread-safe, so this
// class provides the only access point.
//
// Usage:
//   auto* svc = new VideoDecodeService(path);
//   connect(svc, &VideoDecodeService::frameReady, widget, &Widget::onFrame,
//           Qt::QueuedConnection);   // <-- required: cross-thread delivery
//   svc->start();
//   svc->requestFrame(42);
//   // ...
//   svc->stopService();
//   svc->wait();
class VideoDecodeService : public QThread {
    Q_OBJECT

public:
    explicit VideoDecodeService(const QString& videoPath, QObject* parent = nullptr);
    ~VideoDecodeService() override;

    // Request decoding of a specific frame. Called from the UI thread.
    // If multiple calls arrive before the decode thread processes them,
    // only the latest request is honored (latest-only queue).
    void requestFrame(int frameNumber);

    // Gracefully signal the decode thread to exit its loop, then return.
    void stopService();

    bool  isOpen()       const { return captureOpen_; }
    int   totalFrames()  const { return totalFrames_; }
    double fps()         const { return fps_; }
    QSize resolution()   const { return resolution_; }

signals:
    // Emitted from the decode thread via Qt::QueuedConnection → UI thread.
    void frameReady(QImage frame, int frameNumber);
    void videoMetadataReady(int totalFrames, double fps, QSize resolution);
    void decodeError(const QString& message);

protected:
    // QThread entry point. Runs the decode loop until stopService() is called.
    void run() override;

private:
    QImage decodeFrame(int frameNumber);
    static QImage matToQImage(const cv::Mat& mat);

    QString           videoPath_;
    cv::VideoCapture  capture_;
    bool              captureOpen_{false};
    int               totalFrames_{0};
    double            fps_{30.0};
    QSize             resolution_;

    FrameCache        cache_{60};     // 60-frame LRU cache
    FrameRequestQueue requestQueue_;
    std::atomic<bool> running_{false};
};
