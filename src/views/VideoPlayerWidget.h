#pragma once
#include <QWidget>
#include <QImage>
#include <QRectF>
#include "../models/PoseFrame.h"
#include "../models/BiomechanicsMetrics.h"
#include "PoseOverlayRenderer.h"

// VideoPlayerWidget is the main video display surface.
//
// Each paintEvent():
//   1. Fills background with black.
//   2. Blits the current QImage frame into a letterboxed rect.
//   3. If pose data is available and visible, calls PoseOverlayRenderer::render().
//
// Frame updates come from VideoDecodeService via Qt::QueuedConnection.
// This widget never decodes frames — it only displays them.
class VideoPlayerWidget : public QWidget {
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget* parent = nullptr);

    // Set the source video dimensions (used for letterboxing calculation)
    void setVideoSize(int width, int height);

    // Toggle pose skeleton overlay visibility
    void setPoseVisible(bool visible) { poseVisible_ = visible; update(); }
    bool isPoseVisible() const { return poseVisible_; }

    QRectF currentDestRect() const { return destRect_; }

public slots:
    // Called from VideoDecodeService via QueuedConnection
    void onFrameReady(QImage frame, int frameNumber);

    // Called when analysis completes — provides the pose for the current frame
    void setPoseFrame(const PoseFrame& frame, PitchPhase phase = PitchPhase::Unknown);

    void clearFrame();

signals:
    void clicked();                          // left-click → toggle play/pause
    void exportFrameRequested(int frameNumber);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    QSize sizeHint() const override { return {640, 360}; }

private:
    void updateDestRect();

    QImage            currentFrame_;
    int               currentFrameNumber_{-1};
    PoseFrame         currentPose_;
    PitchPhase        currentPhase_{PitchPhase::Unknown};
    bool              poseVisible_{true};

    int               videoWidth_{640};
    int               videoHeight_{360};
    QRectF            destRect_;

    PoseOverlayRenderer overlayRenderer_;
};
