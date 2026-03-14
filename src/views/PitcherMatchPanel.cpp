#include "PitcherMatchPanel.h"
#include <QFrame>
#include <QHBoxLayout>

PitcherMatchPanel::PitcherMatchPanel(QWidget* parent)
    : QDockWidget(QStringLiteral("Pro Matches"), parent) {
    setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    setMinimumWidth(220);

    container_ = new QWidget(this);
    container_->setStyleSheet(QStringLiteral("background: #1a1a22;"));
    layout_ = new QVBoxLayout(container_);
    layout_->setContentsMargins(6, 10, 6, 10);
    layout_->setSpacing(6);

    auto* placeholder = new QLabel(
        QStringLiteral("Run analysis to see pitcher matches."), container_);
    placeholder->setWordWrap(true);
    placeholder->setStyleSheet(
        QStringLiteral("color: #5a5a7a; font-size: 10px; padding: 8px;"));
    layout_->addWidget(placeholder);
    layout_->addStretch();

    setWidget(container_);
}

void PitcherMatchPanel::setMatches(const std::vector<PitcherMatch>& matches) {
    // Clear old widgets
    QLayoutItem* item;
    while ((item = layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    if (matches.empty()) {
        clearMatches();
        return;
    }

    for (const auto& match : matches) {
        // Card frame
        auto* card = new QFrame(container_);
        card->setFrameShape(QFrame::StyledPanel);
        card->setStyleSheet(
            QStringLiteral("QFrame { background: #22222e; border: 1px solid #333348;"
                           " border-radius: 6px; }"));

        auto* cardLayout = new QVBoxLayout(card);
        cardLayout->setContentsMargins(10, 8, 10, 8);
        cardLayout->setSpacing(4);

        // Pitcher name
        auto* nameLabel = new QLabel(match.profile.name, card);
        nameLabel->setStyleSheet(
            QStringLiteral("color: #e0e0f0; font-weight: bold; font-size: 12px;"
                           " border: none; background: transparent;"));
        cardLayout->addWidget(nameLabel);

        // Similarity score
        int pct = static_cast<int>(match.similarityScore * 100);
        QColor scoreColor = pct >= 80 ? QColor(0x4C, 0xAF, 0x50)
                          : pct >= 60 ? QColor(0xFF, 0xC1, 0x07)
                          : QColor(0xF4, 0x43, 0x36);
        auto* scoreLabel = new QLabel(
            QStringLiteral("Similarity: %1%").arg(pct), card);
        scoreLabel->setStyleSheet(
            QStringLiteral("color: %1; font-size: 11px; border: none; background: transparent;")
                .arg(scoreColor.name()));
        cardLayout->addWidget(scoreLabel);

        // Coaching cues (first 2)
        for (int i = 0; i < std::min<int>(2, match.coachingCues.size()); ++i) {
            auto* cueLabel = new QLabel(
                QStringLiteral("• ") + match.coachingCues[i], card);
            cueLabel->setWordWrap(true);
            cueLabel->setStyleSheet(
                QStringLiteral("color: #9090b0; font-size: 9px;"
                               " border: none; background: transparent;"));
            cardLayout->addWidget(cueLabel);
        }

        // Compare button
        auto* compareBtn = new QPushButton(QStringLiteral("Compare Side-by-Side"), card);
        compareBtn->setStyleSheet(
            QStringLiteral("QPushButton { background: #2a4a7a; color: #80c0ff;"
                           " border: 1px solid #3a5a9a; border-radius: 4px;"
                           " padding: 4px 8px; font-size: 10px; }"
                           "QPushButton:hover { background: #3a5a9a; }"));
        QString id = match.profile.id;
        connect(compareBtn, &QPushButton::clicked, this, [this, id] {
            emit compareRequested(id);
        });
        cardLayout->addWidget(compareBtn);

        layout_->addWidget(card);
    }

    layout_->addStretch();
}

void PitcherMatchPanel::clearMatches() {
    QLayoutItem* item;
    while ((item = layout_->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }
    auto* placeholder = new QLabel(
        QStringLiteral("Run analysis to see pitcher matches."), container_);
    placeholder->setWordWrap(true);
    placeholder->setStyleSheet(
        QStringLiteral("color: #5a5a7a; font-size: 10px; padding: 8px;"));
    layout_->addWidget(placeholder);
    layout_->addStretch();
}
