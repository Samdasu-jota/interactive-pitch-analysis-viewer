#pragma once
#include <QPainter>
#include <QRectF>
#include <QString>

// Strategy interface for rendering a single metric visualization inside a MetricCard.
// Each metric type (angle, ratio, tempo) has its own implementation.
class IMetricRenderer {
public:
    virtual ~IMetricRenderer() = default;

    // Draw the metric gauge/visualization inside gaugeRect.
    // value is the raw metric value. The renderer decides how to map it.
    virtual void renderGauge(QPainter& painter, const QRectF& gaugeRect,
                              double value) const = 0;

    // Format value for display (e.g., "87°", "0.92", "0.65s")
    virtual QString formatValue(double value) const = 0;

    // Short label for the metric (e.g., "Elbow Angle")
    virtual QString label() const = 0;

    // Optional: description shown in tooltip
    virtual QString description() const { return {}; }
};
