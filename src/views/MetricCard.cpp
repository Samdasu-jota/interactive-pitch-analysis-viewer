#include "MetricCard.h"
#include <QPainter>
#include <QFont>
#include <QEasingCurve>

MetricCard::MetricCard(std::unique_ptr<IMetricRenderer> renderer, QWidget* parent)
    : QFrame(parent)
    , renderer_(std::move(renderer))
    , animation_(new QPropertyAnimation(this, "displayValue", this)) {
    setMinimumHeight(70);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setFrameShape(QFrame::NoFrame);

    animation_->setDuration(300);
    animation_->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation_, &QPropertyAnimation::valueChanged,
            this, [this] { update(); });
}

void MetricCard::setDisplayValue(double v) {
    displayValue_ = v;
    update();
}

void MetricCard::setValue(double value) {
    targetValue_ = value;
    animation_->stop();
    animation_->setStartValue(displayValue_);
    animation_->setEndValue(value);
    animation_->start();
}

void MetricCard::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const QRect r = rect().adjusted(8, 6, -8, -6);

    // Label
    QFont labelFont;
    labelFont.setPointSizeF(8.5);
    labelFont.setBold(true);
    p.setFont(labelFont);
    p.setPen(QColor(160, 165, 185));
    QRect labelRect(r.left(), r.top(), r.width() * 3 / 4, 18);
    p.drawText(labelRect, Qt::AlignVCenter | Qt::AlignLeft, renderer_->label());

    // Value text (right-aligned)
    QFont valueFont;
    valueFont.setPointSizeF(13.0);
    valueFont.setBold(true);
    p.setFont(valueFont);
    p.setPen(QColor(230, 235, 255));
    QRect valueRect(r.left(), r.top(), r.width(), 18);
    p.drawText(valueRect, Qt::AlignVCenter | Qt::AlignRight,
               renderer_->formatValue(displayValue_));

    // Gauge bar
    const int gaugeH = 10;
    QRectF gaugeRect(r.left(), r.top() + 26, r.width(), gaugeH);
    renderer_->renderGauge(p, gaugeRect, displayValue_);
}
