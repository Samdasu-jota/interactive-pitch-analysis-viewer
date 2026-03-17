#include "SpeedControlWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QApplication>
#include <cmath>

SpeedControlWidget::SpeedControlWidget(double initialSpeed, QWidget* parent)
    : QWidget(parent, Qt::Popup)
    , speed_(initialSpeed) {
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(340);

    setStyleSheet(QStringLiteral(
        "SpeedControlWidget {"
        "  background: #1e1e2e;"
        "  border: 1px solid #3a3a5c;"
        "  border-radius: 12px;"
        "}"
        "QLabel#speedLabel {"
        "  color: #ffffff;"
        "  font-size: 18px;"
        "  font-weight: bold;"
        "}"
        "QPushButton#stepBtn {"
        "  background: #2e2e4e;"
        "  color: #ffffff;"
        "  border: none;"
        "  border-radius: 16px;"
        "  font-size: 16px;"
        "  min-width: 32px;"
        "  max-width: 32px;"
        "  min-height: 32px;"
        "  max-height: 32px;"
        "}"
        "QPushButton#stepBtn:hover { background: #3e3e6e; }"
        "QPushButton#presetBtn {"
        "  background: #2e2e4e;"
        "  color: #cccccc;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 6px 4px;"
        "  font-size: 13px;"
        "}"
        "QPushButton#presetBtn:hover { background: #3e3e6e; color: #ffffff; }"
        "QPushButton#presetBtn[active=true] {"
        "  background: #4a90d9;"
        "  color: #ffffff;"
        "}"
        "QSlider::groove:horizontal {"
        "  height: 4px;"
        "  background: #3a3a5c;"
        "  border-radius: 2px;"
        "}"
        "QSlider::handle:horizontal {"
        "  background: #ffffff;"
        "  width: 18px; height: 18px;"
        "  border-radius: 9px;"
        "  margin: -7px 0;"
        "}"
        "QSlider::sub-page:horizontal { background: #4a90d9; border-radius: 2px; }"
    ));

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 14, 16, 14);
    root->setSpacing(12);

    // Speed label
    speedLabel_ = new QLabel(QStringLiteral("1.00x"), this);
    speedLabel_->setObjectName(QStringLiteral("speedLabel"));
    speedLabel_->setAlignment(Qt::AlignCenter);
    root->addWidget(speedLabel_);

    // Slider row
    auto* sliderRow = new QHBoxLayout;
    sliderRow->setSpacing(8);

    auto* minusBtn = new QPushButton(QStringLiteral("−"), this);
    minusBtn->setObjectName(QStringLiteral("stepBtn"));
    sliderRow->addWidget(minusBtn);

    slider_ = new QSlider(Qt::Horizontal, this);
    slider_->setRange(25, 300);   // 0.25x – 3.0x in units of 0.01
    slider_->setSingleStep(25);
    slider_->setPageStep(25);
    sliderRow->addWidget(slider_, 1);

    auto* plusBtn = new QPushButton(QStringLiteral("+"), this);
    plusBtn->setObjectName(QStringLiteral("stepBtn"));
    sliderRow->addWidget(plusBtn);

    root->addLayout(sliderRow);

    // Preset buttons
    struct Preset { double speed; const char* label; const char* sub; };
    const Preset presets[] = {
        {1.00, "1.0",  "Normal"},
        {1.25, "1.25", nullptr},
        {1.50, "1.5",  nullptr},
        {1.75, "1.75", nullptr},
        {2.00, "2.0",  nullptr},
        {3.00, "3.0",  nullptr},
    };

    auto* presetRow = new QHBoxLayout;
    presetRow->setSpacing(6);
    for (const auto& p : presets) {
        auto* col = new QVBoxLayout;
        col->setSpacing(2);

        auto* btn = new QPushButton(QString::fromUtf8(p.label), this);
        btn->setObjectName(QStringLiteral("presetBtn"));
        col->addWidget(btn);

        if (p.sub) {
            auto* sub = new QLabel(QString::fromUtf8(p.sub), this);
            sub->setAlignment(Qt::AlignCenter);
            sub->setStyleSheet(QStringLiteral("color: #888888; font-size: 10px;"));
            col->addWidget(sub);
        }

        presetRow->addLayout(col);

        const double spd = p.speed;
        connect(btn, &QPushButton::clicked, this, [this, spd] { applySpeed(spd); });
    }
    root->addLayout(presetRow);

    // Wire controls
    connect(slider_, &QSlider::valueChanged, this, [this](int v) {
        applySpeed(v / 100.0);
    });
    connect(minusBtn, &QPushButton::clicked, this, [this] {
        applySpeed(std::max(0.25, std::round((speed_ - 0.25) * 4) / 4.0));
    });
    connect(plusBtn, &QPushButton::clicked, this, [this] {
        applySpeed(std::min(3.0, std::round((speed_ + 0.25) * 4) / 4.0));
    });

    // Set initial state (block slider signal to avoid re-emit)
    applySpeed(speed_);
}

void SpeedControlWidget::applySpeed(double speed) {
    speed_ = std::clamp(speed, 0.25, 3.0);

    const bool blocked = slider_->blockSignals(true);
    slider_->setValue(static_cast<int>(std::round(speed_ * 100)));
    slider_->blockSignals(blocked);

    speedLabel_->setText(QStringLiteral("%1x").arg(speed_, 0, 'f', 2));
    emit speedChanged(speed_);
}
