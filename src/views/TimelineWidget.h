#pragma once
#include <QWidget>
#include <QImage>
#include <QList>
#include <QPixmap>
#include "../models/BiomechanicsMetrics.h"

// TimelineWidget provides:
//   - A scrubable playhead bar (click or drag to seek)
//   - A thumbnail strip (1 frame per second)
//   - Colored tick marks at key pitching phase events
//   - Time/frame counter display
//
// This widget only emits signals — it has no knowledge of VideoDecodeService.
// The presenter wires seekRequested() → VideoDecodeService::requestFrame().
class TimelineWidget : public QWidget {
    Q_OBJECT

public:
    explicit TimelineWidget(QWidget* parent = nullptr);

    void setTotalFrames(int frames, double fps);
    void setPhaseFrames(const PhaseFrames& phases);

public slots:
    void setCurrentFrame(int frameNumber);
    void onThumbnailReady(int index, QImage thumbnail);

signals:
    void seekRequested(int frameNumber);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    QSize sizeHint() const override { return {800, 80}; }

private:
    int  pixelToFrame(int x) const;
    int  frameToPixel(int frame) const;
    void paintThumbnailStrip(QPainter& p, const QRect& stripRect);
    void paintPhaseMarkers(QPainter& p, const QRect& scrubRect);
    void paintPlayhead(QPainter& p, const QRect& scrubRect);
    void paintTimeLabel(QPainter& p);

    int         totalFrames_{0};
    double      fps_{30.0};
    int         currentFrame_{0};
    bool        dragging_{false};

    PhaseFrames phases_;

    QList<QPixmap> thumbnails_;

    static constexpr int kThumbnailHeight = 44;
    static constexpr int kScrubHeight     = 16;
    static constexpr int kScrubMargin     = 8;
};
