#pragma once
#include "IMetricRenderer.h"
#include "../utils/MetricThresholds.h"

// Renders a timing/tempo metric (delivery tempo as a 0–1 fraction).
class TempoMetricRenderer : public IMetricRenderer {
public:
    explicit TempoMetricRenderer(const QString& metricKey,
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
