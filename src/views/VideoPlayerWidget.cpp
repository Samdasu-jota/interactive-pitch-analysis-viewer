#include "VideoPlayerWidget.h"
#include "../utils/CoordMapper.h"
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QContextMenuEvent>
#include <QMenu>

VideoPlayerWidget::VideoPlayerWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumSize(320, 180);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(QStringLiteral("background-color: #0a0a0a;"));
    updateDestRect();
}

void VideoPlayerWidget::setVideoSize(int width, int height) {
    videoWidth_  = width  > 0 ? width  : 640;
    videoHeight_ = height > 0 ? height : 360;
    updateDestRect();
}

void VideoPlayerWidget::onFrameReady(QImage frame, int frameNumber) {
    currentFrame_       = std::move(frame);
    currentFrameNumber_ = frameNumber;
    update();  // schedule repaint on next event loop iteration
}

void VideoPlayerWidget::setPoseFrame(const PoseFrame& frame, PitchPhase phase) {
    currentPose_  = frame;
    currentPhase_ = phase;
    update();
}

void VideoPlayerWidget::clearFrame() {
    currentFrame_       = {};
    currentFrameNumber_ = -1;
    currentPose_        = {};
    currentPhase_       = PitchPhase::Unknown;
    update();
}

// ── paintEvent ────────────────────────────────────────────────────────────────

void VideoPlayerWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 1. Black background
    painter.fillRect(rect(), QColor(10, 10, 10));

    if (currentFrame_.isNull()) {
        // Placeholder when no video loaded
        painter.setPen(QColor(60, 60, 70));
        QFont font;
        font.setPointSize(11);
        painter.setFont(font);
        painter.drawText(rect(), Qt::AlignCenter,
                         QStringLiteral("Open a video to begin"));
        return;
    }

    // 2. Draw video frame letterboxed into destRect_
    painter.drawImage(destRect_, currentFrame_);

    // 3. Pose overlay (if analysis has been run)
    if (poseVisible_ && currentPose_.isValid()) {
        overlayRenderer_.render(painter, currentPose_, destRect_, currentPhase_);
    }
}

void VideoPlayerWidget::resizeEvent(QResizeEvent* /*event*/) {
    updateDestRect();
}

void VideoPlayerWidget::updateDestRect() {
    destRect_ = CoordMapper::letterboxRect(
        videoWidth_, videoHeight_, width(), height());
}

void VideoPlayerWidget::contextMenuEvent(QContextMenuEvent* event) {
    if (currentFrameNumber_ < 0) return;

    QMenu menu(this);
    QAction* exportAct = menu.addAction(QStringLiteral("Export Annotated Frame..."));
    connect(exportAct, &QAction::triggered, this, [this] {
        emit exportFrameRequested(currentFrameNumber_);
    });
    menu.exec(event->globalPos());
}
