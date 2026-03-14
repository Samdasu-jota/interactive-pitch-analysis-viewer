#include "AngleMetricRenderer.h"
#include <QPen>
#include <QBrush>
#include <QLinearGradient>
#include <cmath>

AngleMetricRenderer::AngleMetricRenderer(const QString& metricKey,
                                          const QString& label,
                                          const QString& description)
    : label_(label)
    , description_(description)
    , range_(MetricThresholds::getRange(metricKey)) {}

void AngleMetricRenderer::renderGauge(QPainter& painter,
                                       const QRectF& gaugeRect,
                                       double value) const {
    const double fill = range_.normalize(value);
    const QColor color = range_.colorFor(value);

    // Track background
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(40, 40, 50));
    painter.drawRoundedRect(gaugeRect, 3, 3);

    // Filled portion
    QRectF filled = gaugeRect;
    filled.setWidth(gaugeRect.width() * fill);
    if (filled.width() > 0) {
        QLinearGradient grad(filled.left(), 0, filled.right(), 0);
        grad.setColorAt(0.0, color.darker(110));
        grad.setColorAt(1.0, color);
        painter.setBrush(grad);
        painter.drawRoundedRect(filled, 3, 3);
    }

    // Optimal zone indicator (subtle lighter band)
    double optStart = range_.normalize(range_.optimalMin);
    double optEnd   = range_.normalize(range_.optimalMax);
    QRectF optZone  = gaugeRect;
    optZone.setLeft(gaugeRect.left() + optStart * gaugeRect.width());
    optZone.setRight(gaugeRect.left() + optEnd   * gaugeRect.width());
    painter.setBrush(QColor(255, 255, 255, 18));
    painter.drawRoundedRect(optZone, 2, 2);
}

QString AngleMetricRenderer::formatValue(double value) const {
    return QStringLiteral("%1%2")
        .arg(static_cast<int>(std::round(value)))
        .arg(range_.unit.isEmpty() ? QStringLiteral("°") : range_.unit);
}
