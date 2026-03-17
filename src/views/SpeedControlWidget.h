#pragma once
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QPushButton>

// Popup widget for playback speed control.
// Shows current speed, a slider with -/+ buttons, and preset speed buttons.
// Emits speedChanged(double) when the user selects a new speed.
class SpeedControlWidget : public QWidget {
    Q_OBJECT

public:
    explicit SpeedControlWidget(double initialSpeed = 1.0, QWidget* parent = nullptr);

signals:
    void speedChanged(double speed);

private:
    void applySpeed(double speed);

    QLabel*  speedLabel_{nullptr};
    QSlider* slider_{nullptr};
    double   speed_{1.0};
};
