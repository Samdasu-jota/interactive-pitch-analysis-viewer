#include "TempoMetricRenderer.h"
#include <QPen>
#include <QBrush>
#include <QLinearGradient>

TempoMetricRenderer::TempoMetricRenderer(const QString& metricKey,
                                          const QString& label,
                                          const QString& description)
    : label_(label)
    , description_(description)
    , range_(MetricThresholds::getRange(metricKey)) {}

void TempoMetricRenderer::renderGauge(QPainter& painter,
                                       const QRectF& gaugeRect,
                                       double value) const {
    const double fill  = range_.normalize(value);
    const QColor color = range_.colorFor(value);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(40, 40, 50));
    painter.drawRoundedRect(gaugeRect, 3, 3);

    QRectF filled = gaugeRect;
    filled.setWidth(gaugeRect.width() * fill);
    if (filled.width() > 0) {
        painter.setBrush(color);
        painter.drawRoundedRect(filled, 3, 3);
    }

    // Draw tick at optimal midpoint
    double optMid = (range_.optimalMin + range_.optimalMax) / 2.0;
    double tickX  = gaugeRect.left() + range_.normalize(optMid) * gaugeRect.width();
    painter.setPen(QPen(QColor(255, 255, 255, 80), 1));
    painter.drawLine(QPointF(tickX, gaugeRect.top()),
                     QPointF(tickX, gaugeRect.bottom()));
}

QString TempoMetricRenderer::formatValue(double value) const {
    return QStringLiteral("%1").arg(value, 0, 'f', 2);
}
