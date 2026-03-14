#pragma once
#include <QPainter>
#include <QRectF>
#include <QColor>
#include "../models/PoseFrame.h"
#include "../models/BiomechanicsMetrics.h"

// Stateless renderer for the MediaPipe Pose skeleton overlay.
//
// PoseOverlayRenderer has NO member state — all drawing parameters are passed
// as arguments. This makes it trivially thread-safe (though rendering always
// happens on the UI thread via paintEvent).
//
// Rendering layers (in order):
//   1. Bone lines (connections between keypoints)
//   2. Joint circles (at each visible keypoint)
//   3. Angle annotations at elbow, shoulder, hip
//   4. Phase label in corner
class PoseOverlayRenderer {
public:
    struct Style {
        float  minVisibility{0.5f};    // keypoints below this are skipped
        int    boneWidth{2};
        int    jointRadius{4};
        QColor boneColor{0, 230, 230, 200};       // cyan
        QColor armBoneColor{255, 165, 0, 220};    // orange
        QColor legBoneColor{100, 200, 255, 200};  // light blue
        QColor jointColor{255, 255, 255, 230};    // white
        QColor annotationBg{0, 0, 0, 160};        // semi-transparent black
        QColor annotationText{255, 255, 255};
        QColor phaseTextColor{255, 220, 0};       // gold
        bool   showAngles{true};
        bool   showPhaseLabel{true};
    };

    PoseOverlayRenderer() = default;
    explicit PoseOverlayRenderer(Style style);

    // Render the pose overlay onto an active QPainter.
    // destRect is the letterboxed area where the video frame is drawn —
    // keypoint coordinates are mapped relative to this rect.
    void render(QPainter& painter,
                const PoseFrame& frame,
                const QRectF& destRect,
                PitchPhase phase = PitchPhase::Unknown) const;

    Style& style() { return style_; }

private:
    void drawBones(QPainter& painter,
                   const std::array<QPointF, 33>& pts,
                   const std::array<bool, 33>& visible) const;

    void drawJoints(QPainter& painter,
                    const std::array<QPointF, 33>& pts,
                    const std::array<bool, 33>& visible) const;

    void drawAngleAnnotation(QPainter& painter,
                              QPointF joint, QPointF ptA, QPointF ptB,
                              double angleDeg) const;

    void drawAngles(QPainter& painter,
                    const std::array<QPointF, 33>& pts,
                    const std::array<bool, 33>& visible,
                    const PoseFrame& frame) const;

    void drawPhaseLabel(QPainter& painter,
                        const QRectF& destRect,
                        PitchPhase phase) const;

    static double computeAngle(QPointF a, QPointF b, QPointF c);

    Style style_;
};
