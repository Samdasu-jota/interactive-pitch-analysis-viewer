#include "RatioMetricRenderer.h"
#include <QPen>
#include <QBrush>
#include <QLinearGradient>

RatioMetricRenderer::RatioMetricRenderer(const QString& metricKey,
                                          const QString& label,
                                          const QString& description)
    : label_(label)
    , description_(description)
    , range_(MetricThresholds::getRange(metricKey)) {}

void RatioMetricRenderer::renderGauge(QPainter& painter,
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
        QLinearGradient grad(filled.left(), 0, filled.right(), 0);
        grad.setColorAt(0.0, color.darker(115));
        grad.setColorAt(1.0, color);
        painter.setBrush(grad);
        painter.drawRoundedRect(filled, 3, 3);
    }
}

QString RatioMetricRenderer::formatValue(double value) const {
    if (range_.unit.isEmpty()) {
        return QString::number(value, 'f', 2);
    }
    return QStringLiteral("%1 %2").arg(value, 0, 'f', 2).arg(range_.unit);
}
