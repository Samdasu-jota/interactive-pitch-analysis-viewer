#pragma once
#include <QPointF>
#include <QRectF>
#include "../models/PoseLandmark.h"

// Maps normalized MediaPipe pose coordinates [0.0, 1.0] to widget pixel coords.
//
// The destRect is the letterboxed rectangle within the VideoPlayerWidget where
// the actual video frame is painted. Keypoints are mapped relative to that rect,
// not the full widget size.
class CoordMapper {
public:
    // Map a single landmark's (x, y) into widget pixel coordinates.
    // x and y are normalized [0.0, 1.0] relative to the source video frame.
    static QPointF toWidget(const PoseLandmark& lm, const QRectF& destRect) {
        return {
            destRect.left() + lm.x * destRect.width(),
            destRect.top()  + lm.y * destRect.height()
        };
    }

    // Compute the letterbox rect that preserves aspect ratio within widgetSize.
    // Used by VideoPlayerWidget to know where the frame pixels actually land.
    static QRectF letterboxRect(int frameW, int frameH,
                                int widgetW, int widgetH) {
        if (frameW <= 0 || frameH <= 0) return {0, 0, (double)widgetW, (double)widgetH};

        double frameAspect  = static_cast<double>(frameW) / frameH;
        double widgetAspect = static_cast<double>(widgetW) / widgetH;

        double drawW, drawH;
        if (frameAspect > widgetAspect) {
            drawW = widgetW;
            drawH = widgetW / frameAspect;
        } else {
            drawH = widgetH;
            drawW = widgetH * frameAspect;
        }

        double x = (widgetW - drawW) / 2.0;
        double y = (widgetH - drawH) / 2.0;
        return {x, y, drawW, drawH};
    }
};
