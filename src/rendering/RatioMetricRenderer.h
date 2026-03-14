#pragma once
#include "IMetricRenderer.h"
#include "../utils/MetricThresholds.h"

// Renders a ratio / normalized metric (0.0–1.0 or 0.0–1.2 range).
class RatioMetricRenderer : public IMetricRenderer {
public:
    explicit RatioMetricRenderer(const QString& metricKey,
                                  const QString& label,
                                  const QString& description = {});

    void    renderGauge(QPainter& painter, const QRectF& gaugeRect,
                        double value) const override;
    QString formatValue(double value)  const override;
    QString label()                    const override { return label_; }
    QString description()              const override { return description_; }

private:
    QString     label_;
    QString     description_;
    MetricRange range_;
};
