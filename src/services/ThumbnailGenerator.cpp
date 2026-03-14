#include "ThumbnailGenerator.h"
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <QDebug>

ThumbnailGenerator::ThumbnailGenerator(QObject* parent)
    : QObject(parent) {}

void ThumbnailGenerator::generate(const QString& videoPath, double fps,
                                   int totalFrames, int thumbnailWidth) {
    cancelled_.store(false);

    // Capture by value for the lambda (runs on a thread pool thread)
    QtConcurrent::run([=]() {
        cv::VideoCapture cap(videoPath.toStdString());
        if (!cap.isOpened()) {
            qWarning() << "ThumbnailGenerator: cannot open" << videoPath;
            emit generationCancelled();
            return;
        }

        const int framesPerThumb = std::max(1, static_cast<int>(fps));
        int thumbIndex = 0;

        for (int frame = 0; frame < totalFrames; frame += framesPerThumb) {
            if (cancelled_.load()) {
                emit generationCancelled();
                cap.release();
                return;
            }

            cap.set(cv::CAP_PROP_POS_FRAMES, static_cast<double>(frame));
            cv::Mat bgr;
            if (!cap.read(bgr) || bgr.empty()) continue;

            // Scale to thumbnail width, preserve aspect ratio
            int thumbH = static_cast<int>(
                thumbnailWidth * static_cast<double>(bgr.rows) / bgr.cols);
            cv::Mat resized;
            cv::resize(bgr, resized, cv::Size(thumbnailWidth, thumbH));

            cv::Mat rgb;
            cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);

            QImage thumb(rgb.data, rgb.cols, rgb.rows,
                         static_cast<int>(rgb.step), QImage::Format_RGB888);

            emit thumbnailReady(thumbIndex++, thumb.copy());
        }

        cap.release();
        emit generationComplete(thumbIndex);
    });
}

void ThumbnailGenerator::cancel() {
    cancelled_.store(true);
}
