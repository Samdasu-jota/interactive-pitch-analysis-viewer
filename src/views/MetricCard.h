#pragma once
#include <QFrame>
#include <QPropertyAnimation>
#include <memory>
#include "../rendering/IMetricRenderer.h"

// MetricCard displays a single biomechanical metric with:
//   - Label (e.g., "Elbow Angle")
//   - Animated gauge bar (color-coded green/amber/red)
//   - Value text (e.g., "87°")
//
// The gauge bar width animates via QPropertyAnimation when the value changes,
// providing smooth visual feedback — exactly like Tesla's energy display transitions.
class MetricCard : public QFrame {
    Q_OBJECT
    Q_PROPERTY(double displayValue READ displayValue WRITE setDisplayValue)

public:
    explicit MetricCard(std::unique_ptr<IMetricRenderer> renderer,
                        QWidget* parent = nullptr);

    double displayValue() const { return displayValue_; }
    void   setDisplayValue(double v);

public slots:
    void setValue(double value);

protected:
    void paintEvent(QPaintEvent* event) override;
    QSize sizeHint() const override { return {200, 72}; }

private:
    std::unique_ptr<IMetricRenderer> renderer_;
    double                           targetValue_{0.0};
    double                           displayValue_{0.0};
    QPropertyAnimation*              animation_;
};
