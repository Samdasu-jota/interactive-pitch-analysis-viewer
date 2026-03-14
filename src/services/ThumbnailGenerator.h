#pragma once
#include <QObject>
#include <QImage>
#include <QString>

// Generates a strip of video thumbnails for the TimelineWidget.
// Runs asynchronously using QtConcurrent::run — does NOT block the UI thread.
//
// One thumbnail is sampled per second of video. Results are delivered
// one-at-a-time via thumbnailReady() as each is decoded.
class ThumbnailGenerator : public QObject {
    Q_OBJECT

public:
    explicit ThumbnailGenerator(QObject* parent = nullptr);

    // Start generating thumbnails for the given video.
    // thumbnailSize controls the width of each thumbnail (height is proportional).
    void generate(const QString& videoPath, double fps, int totalFrames,
                  int thumbnailWidth = 80);

    void cancel();

signals:
    void thumbnailReady(int index, QImage thumbnail);
    void generationComplete(int totalThumbnails);
    void generationCancelled();

private:
    std::atomic<bool> cancelled_{false};
};
