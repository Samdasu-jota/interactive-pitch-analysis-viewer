#pragma once
#include "IMetricRenderer.h"
#include "../utils/MetricThresholds.h"

// Renders an angle metric (measured in degrees) as a horizontal gauge bar
// with a color-coded fill: green (optimal) → amber (warning) → red (poor).
class AngleMetricRenderer : public IMetricRenderer {
public:
    explicit AngleMetricRenderer(const QString& metricKey,
                                  const QString& label,
                                  const QString& description = {});

    void    renderGauge(QPainter& painter, const QRectF& gaugeRect,
                        double value) const override;
    QString formatValue(double value)  const override;
    QString label()                    const override { return label_; }
    QString description()              const override { return description_; }

private:
    QString      label_;
    QString      description_;
    MetricRange  range_;
};
