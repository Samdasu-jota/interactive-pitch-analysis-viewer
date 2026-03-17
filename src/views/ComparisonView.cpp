#include "ComparisonView.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>

ComparisonView::ComparisonView(QWidget* parent)
    : QWidget(parent) {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Left: user player ────────────────────────────────────────────────────
    auto* leftContainer = new QWidget(this);
    auto* leftLayout    = new QVBoxLayout(leftContainer);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(2);

    userLabel_ = new QLabel(QStringLiteral("VIDEO A  —  Click to open"), leftContainer);
    userLabel_->setStyleSheet(
        QStringLiteral("color: #a0c0e0; font-weight: bold; font-size: 11px;"
                       " padding: 4px 8px; background: #1a1a28;"));
    userLabel_->setAlignment(Qt::AlignCenter);

    userPlayer_   = new VideoPlayerWidget(leftContainer);
    userTimeline_ = new TimelineWidget(leftContainer);
    leftLayout->addWidget(userLabel_);
    leftLayout->addWidget(userPlayer_, 1);
    leftLayout->addWidget(userTimeline_);

    // ── Right: comparison player ─────────────────────────────────────────────
    auto* rightContainer = new QWidget(this);
    auto* rightLayout    = new QVBoxLayout(rightContainer);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(2);

    compLabel_ = new QLabel(QStringLiteral("VIDEO B  —  Click to open"), rightContainer);
    compLabel_->setStyleSheet(userLabel_->styleSheet());
    compLabel_->setAlignment(Qt::AlignCenter);

    compPlayer_   = new VideoPlayerWidget(rightContainer);
    compTimeline_ = new TimelineWidget(rightContainer);
    rightLayout->addWidget(compLabel_);
    rightLayout->addWidget(compPlayer_, 1);
    rightLayout->addWidget(compTimeline_);

    // Accept drops on the right container so user can drag a video onto it
    rightContainer->setAcceptDrops(true);
    rightContainer->installEventFilter(this);

    // ── Delta panel ──────────────────────────────────────────────────────────
    deltaPanel_ = new QWidget(this);
    deltaPanel_->setFixedWidth(160);
    deltaPanel_->setStyleSheet(QStringLiteral("background: #141420;"));
    auto* deltaLayout = new QVBoxLayout(deltaPanel_);
    deltaLayout->setContentsMargins(8, 12, 8, 12);

    auto* deltaTitle = new QLabel(QStringLiteral("DELTA"), deltaPanel_);
    deltaTitle->setStyleSheet(
        QStringLiteral("color: #6a6a8a; font-size: 9px; font-weight: bold;"
                       " letter-spacing: 1px;"));
    deltaLayout->addWidget(deltaTitle);
    deltaLayout->addStretch();

    // ── Splitter ─────────────────────────────────────────────────────────────
    splitter_ = new QSplitter(Qt::Horizontal, this);
    splitter_->addWidget(leftContainer);
    splitter_->addWidget(rightContainer);
    splitter_->setHandleWidth(4);
    splitter_->setStyleSheet(
        QStringLiteral("QSplitter::handle { background: #333345; }"));

    mainLayout->addWidget(splitter_, 1);
    mainLayout->addWidget(deltaPanel_);
}

void ComparisonView::setUserLabel(const QString& label) {
    userLabel_->setText(label.toUpper());
}

void ComparisonView::setComparisonPitcher(const PitcherMatch& match) {
    int pct = static_cast<int>(match.similarityScore * 100);
    compLabel_->setText(
        QStringLiteral("%1  (%2% match)").arg(match.profile.name.toUpper()).arg(pct));
}

void ComparisonView::setSyncEnabled(bool enabled) {
    syncEnabled_ = enabled;
}

void ComparisonView::onUserFrameAdvanced(int frameNumber) {
    if (syncEnabled_ && !blockSync_) {
        blockSync_ = true;
        emit seekRequested(frameNumber);
        blockSync_ = false;
    }
}

void ComparisonView::onComparisonFrameAdvanced(int /*frameNumber*/) {
    // Comparison player follows user player in sync mode — no reverse sync
}

bool ComparisonView::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::DragEnter) {
        auto* e = static_cast<QDragEnterEvent*>(event);
        if (e->mimeData()->hasUrls()) { e->acceptProposedAction(); return true; }
    }
    if (event->type() == QEvent::Drop) {
        auto* e = static_cast<QDropEvent*>(event);
        const QList<QUrl> urls = e->mimeData()->urls();
        if (!urls.isEmpty()) {
            QString path = urls.first().toLocalFile();
            if (!path.isEmpty()) emit comparisonVideoDropped(path);
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}
