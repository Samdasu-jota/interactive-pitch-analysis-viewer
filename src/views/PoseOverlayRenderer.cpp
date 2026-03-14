#include "PoseOverlayRenderer.h"
#include "../utils/SkeletonTopology.h"
#include "../utils/CoordMapper.h"
#include <QPainterPath>
#include <QFont>
#include <QFontMetricsF>
#include <cmath>

PoseOverlayRenderer::PoseOverlayRenderer(Style style)
    : style_(std::move(style)) {}

// ── Public entry point ────────────────────────────────────────────────────────

void PoseOverlayRenderer::render(QPainter& painter, const PoseFrame& frame,
                                  const QRectF& destRect, PitchPhase phase) const {
    if (!frame.isValid()) return;

    painter.save();

    // 1. Map all 33 keypoints to widget pixel coordinates
    std::array<QPointF, 33> pts;
    std::array<bool, 33>    visible;
    for (int i = 0; i < 33; ++i) {
        visible[i] = (frame.landmarks[i].visibility >= style_.minVisibility);
        pts[i]     = CoordMapper::toWidget(frame.landmarks[i], destRect);
    }

    // 2. Draw skeleton layers
    painter.setRenderHint(QPainter::Antialiasing, true);
    drawBones(painter, pts, visible);
    drawJoints(painter, pts, visible);

    if (style_.showAngles) {
        drawAngles(painter, pts, visible, frame);
    }

    if (style_.showPhaseLabel && phase != PitchPhase::Unknown) {
        drawPhaseLabel(painter, destRect, phase);
    }

    painter.restore();
}

// ── Bones ─────────────────────────────────────────────────────────────────────

void PoseOverlayRenderer::drawBones(QPainter& painter,
                                     const std::array<QPointF, 33>& pts,
                                     const std::array<bool, 33>& visible) const {
    // Draw arm bones in orange, leg bones in light blue, rest in cyan
    auto drawGroup = [&](const auto& connections, const QColor& color) {
        painter.setPen(QPen(color, style_.boneWidth, Qt::SolidLine, Qt::RoundCap));
        for (const auto& [a, b] : connections) {
            if (a < 33 && b < 33 && visible[a] && visible[b]) {
                painter.drawLine(pts[a], pts[b]);
            }
        }
    };

    drawGroup(SkeletonTopology::kConnections,     style_.boneColor);
    drawGroup(SkeletonTopology::kArmConnections,  style_.armBoneColor);
    drawGroup(SkeletonTopology::kLegConnections,  style_.legBoneColor);
    drawGroup(SkeletonTopology::kTorsoConnections,style_.boneColor);
}

// ── Joints ────────────────────────────────────────────────────────────────────

void PoseOverlayRenderer::drawJoints(QPainter& painter,
                                      const std::array<QPointF, 33>& pts,
                                      const std::array<bool, 33>& visible) const {
    painter.setPen(QPen(style_.jointColor, 1));
    painter.setBrush(style_.jointColor);

    for (int i = 0; i < 33; ++i) {
        if (!visible[i]) continue;
        painter.drawEllipse(pts[i],
                            (double)style_.jointRadius,
                            (double)style_.jointRadius);
    }
}

// ── Angle annotations ─────────────────────────────────────────────────────────

// static
double PoseOverlayRenderer::computeAngle(QPointF a, QPointF vertex, QPointF c) {
    QPointF va = a - vertex;
    QPointF vc = c - vertex;
    double dot = va.x() * vc.x() + va.y() * vc.y();
    double magA = std::hypot(va.x(), va.y());
    double magC = std::hypot(vc.x(), vc.y());
    if (magA < 1e-6 || magC < 1e-6) return 0.0;
    double cosAngle = dot / (magA * magC);
    cosAngle = std::max(-1.0, std::min(1.0, cosAngle));  // clamp for acos
    return std::acos(cosAngle) * 180.0 / M_PI;
}

void PoseOverlayRenderer::drawAngleAnnotation(QPainter& painter,
                                               QPointF joint,
                                               QPointF ptA, QPointF ptB,
                                               double angleDeg) const {
    QString label = QStringLiteral("%1°").arg(static_cast<int>(angleDeg));

    QFont font;
    font.setPointSizeF(8.5);
    font.setBold(true);
    painter.setFont(font);

    QFontMetricsF fm(font);
    QRectF bounds = fm.boundingRect(label);
    bounds.adjust(-4, -2, 4, 2);

    // Offset label slightly away from the joint
    QPointF offset(8, -8);
    bounds.moveCenter(joint + offset);

    // Background pill
    painter.setPen(Qt::NoPen);
    painter.setBrush(style_.annotationBg);
    painter.drawRoundedRect(bounds, 3, 3);

    // Text
    painter.setPen(style_.annotationText);
    painter.drawText(bounds, Qt::AlignCenter, label);
}

void PoseOverlayRenderer::drawAngles(QPainter& painter,
                                      const std::array<QPointF, 33>& pts,
                                      const std::array<bool, 33>& visible,
                                      const PoseFrame& /*frame*/) const {
    using namespace LandmarkIndex;

    // Right elbow angle (shoulder → elbow → wrist)
    if (visible[RIGHT_SHOULDER] && visible[RIGHT_ELBOW] && visible[RIGHT_WRIST]) {
        double angle = computeAngle(pts[RIGHT_SHOULDER], pts[RIGHT_ELBOW], pts[RIGHT_WRIST]);
        drawAngleAnnotation(painter, pts[RIGHT_ELBOW],
                            pts[RIGHT_SHOULDER], pts[RIGHT_WRIST], angle);
    }

    // Left elbow angle
    if (visible[LEFT_SHOULDER] && visible[LEFT_ELBOW] && visible[LEFT_WRIST]) {
        double angle = computeAngle(pts[LEFT_SHOULDER], pts[LEFT_ELBOW], pts[LEFT_WRIST]);
        drawAngleAnnotation(painter, pts[LEFT_ELBOW],
                            pts[LEFT_SHOULDER], pts[LEFT_WRIST], angle);
    }

    // Hip-shoulder separation (right side): shoulder–hip–knee
    if (visible[RIGHT_SHOULDER] && visible[RIGHT_HIP] && visible[RIGHT_KNEE]) {
        double angle = computeAngle(pts[RIGHT_SHOULDER], pts[RIGHT_HIP], pts[RIGHT_KNEE]);
        drawAngleAnnotation(painter, pts[RIGHT_HIP],
                            pts[RIGHT_SHOULDER], pts[RIGHT_KNEE], angle);
    }
}

// ── Phase label ───────────────────────────────────────────────────────────────

void PoseOverlayRenderer::drawPhaseLabel(QPainter& painter,
                                          const QRectF& destRect,
                                          PitchPhase phase) const {
    QString text = phaseToString(phase);
    if (text == QLatin1String("—")) return;

    QFont font;
    font.setPointSizeF(11.0);
    font.setBold(true);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 1.5);
    painter.setFont(font);

    QFontMetricsF fm(font);
    QRectF textBounds = fm.boundingRect(text);
    textBounds.adjust(-8, -4, 8, 4);
    textBounds.moveTopLeft(destRect.topLeft() + QPointF(12, 12));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 150));
    painter.drawRoundedRect(textBounds, 4, 4);

    painter.setPen(style_.phaseTextColor);
    painter.drawText(textBounds, Qt::AlignCenter, text);
}
