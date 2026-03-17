#include "MetricsPanel.h"
#include "../rendering/AngleMetricRenderer.h"
#include "../rendering/RatioMetricRenderer.h"
#include "../rendering/TempoMetricRenderer.h"
#include <QLabel>

MetricsPanel::MetricsPanel(QWidget* parent)
    : QDockWidget(QStringLiteral("Biomechanics"), parent) {
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setMinimumWidth(220);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setStyleSheet(QStringLiteral("background: transparent;"));

    container_ = new QWidget(scrollArea);
    container_->setStyleSheet(QStringLiteral("background: #1a1a22;"));
    layout_ = new QVBoxLayout(container_);
    layout_->setContentsMargins(6, 10, 6, 10);
    layout_->setSpacing(4);

    buildCards();
    layout_->addStretch();

    scrollArea->setWidget(container_);
    setWidget(scrollArea);
}

void MetricsPanel::buildCards() {
    // Section header
    auto* header1 = new QLabel(QStringLiteral("ARM MECHANICS"), container_);
    header1->setStyleSheet(
        QStringLiteral("color: #6a6a8a; font-size: 9px; font-weight: bold;"
                       " letter-spacing: 1px; padding-top: 8px;"));
    layout_->addWidget(header1);

    addCard("elbowAngle",
        new MetricCard(std::make_unique<AngleMetricRenderer>(
            "elbowAngle", "Elbow Angle",
            "Shoulder-elbow-wrist angle at release. 85–100° is optimal.")));

    addCard("armExtension",
        new MetricCard(std::make_unique<RatioMetricRenderer>(
            "armExtension", "Arm Extension",
            "Forward reach from shoulder (normalized). Higher = more leverage.")));

    addCard("releaseHeight",
        new MetricCard(std::make_unique<RatioMetricRenderer>(
            "releaseHeight", "Release Height",
            "Vertical hand position at release (normalized body height).")));

    addCard("armSlotHeight",
        new MetricCard(std::make_unique<RatioMetricRenderer>(
            "armSlotHeight", "Arm Slot",
            "Vertical hand position relative to body (normalized). Higher = over-the-top slot.")));

    addCard("releaseExtension",
        new MetricCard(std::make_unique<RatioMetricRenderer>(
            "releaseExtension", "Release Ext.",
            "Forward arm extension at release as fraction of body height. Higher = more reach.")));

    auto* header2 = new QLabel(QStringLiteral("ROTATION"), container_);
    header2->setStyleSheet(header1->styleSheet());
    layout_->addWidget(header2);

    addCard("hipShoulderSeparation",
        new MetricCard(std::make_unique<AngleMetricRenderer>(
            "hipShoulderSeparation", "Hip-Shoulder Sep.",
            "Angular difference between hip and shoulder rotation. 45–65° is elite.")));

    addCard("shoulderRotationAngle",
        new MetricCard(std::make_unique<AngleMetricRenderer>(
            "shoulderRotationAngle", "Shoulder Rotation",
            "Total shoulder rotation at release.")));

    addCard("hipRotationAngle",
        new MetricCard(std::make_unique<AngleMetricRenderer>(
            "hipRotationAngle", "Hip Rotation",
            "Hip rotation angle at release. 60–85° generates maximum rotational power.")));

    auto* header3 = new QLabel(QStringLiteral("STRIDE"), container_);
    header3->setStyleSheet(header1->styleSheet());
    layout_->addWidget(header3);

    addCard("strideLength",
        new MetricCard(std::make_unique<RatioMetricRenderer>(
            "strideLength", "Stride Length",
            "Stride length as fraction of body height. 75–95% is optimal.")));

    addCard("leadKneeFlexion",
        new MetricCard(std::make_unique<AngleMetricRenderer>(
            "leadKneeFlexion", "Lead Knee Flexion",
            "Lead knee bend at foot plant. 30–55° aids deceleration.")));

    addCard("strideDirectionAngle",
        new MetricCard(std::make_unique<AngleMetricRenderer>(
            "strideDirectionAngle", "Stride Angle",
            "Stride direction deviation from home plate line. 0–15° is ideal alignment.")));

    addCard("releaseLateralPosition",
        new MetricCard(std::make_unique<RatioMetricRenderer>(
            "releaseLateralPosition", "Lateral Pos.",
            "Lateral hand position at release (normalized). Near 0 = center-aligned release.")));

    auto* header4 = new QLabel(QStringLiteral("TIMING"), container_);
    header4->setStyleSheet(header1->styleSheet());
    layout_->addWidget(header4);

    addCard("deliveryTempo",
        new MetricCard(std::make_unique<TempoMetricRenderer>(
            "deliveryTempo", "Delivery Tempo",
            "Release timing as fraction of total delivery (0–1). 0.55–0.70 is optimal.")));
}

void MetricsPanel::addCard(const QString& key, MetricCard* card) {
    card->setParent(container_);
    layout_->addWidget(card);
    cards_.insert(key, card);
}

void MetricsPanel::updateMetrics(const BiomechanicsMetrics& m) {
    if (auto* c = cards_.value("elbowAngle"))            c->setValue(m.elbowAngle);
    if (auto* c = cards_.value("armExtension"))          c->setValue(m.armExtension);
    if (auto* c = cards_.value("releaseHeight"))         c->setValue(m.releaseHeight);
    if (auto* c = cards_.value("hipShoulderSeparation")) c->setValue(m.hipShoulderSeparation);
    if (auto* c = cards_.value("shoulderRotationAngle")) c->setValue(m.shoulderRotationAngle);
    if (auto* c = cards_.value("strideLength"))          c->setValue(m.strideLength);
    if (auto* c = cards_.value("leadKneeFlexion"))       c->setValue(m.leadKneeFlexion);
    if (auto* c = cards_.value("deliveryTempo"))              c->setValue(m.deliveryTempo);
    if (auto* c = cards_.value("armSlotHeight"))              c->setValue(m.armSlotHeight);
    if (auto* c = cards_.value("releaseExtension"))           c->setValue(m.releaseExtension);
    if (auto* c = cards_.value("hipRotationAngle"))           c->setValue(m.hipRotationAngle);
    if (auto* c = cards_.value("strideDirectionAngle"))       c->setValue(m.strideDirectionAngle);
    if (auto* c = cards_.value("releaseLateralPosition"))     c->setValue(m.releaseLateralPosition);
}

void MetricsPanel::clearMetrics() {
    for (auto* card : cards_) card->setValue(0.0);
}
