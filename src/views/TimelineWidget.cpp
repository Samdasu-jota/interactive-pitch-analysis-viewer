#include "TimelineWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <algorithm>

TimelineWidget::TimelineWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumHeight(kThumbnailHeight + kScrubHeight + kScrubMargin * 2 + 4);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setStyleSheet(QStringLiteral("background-color: #141418;"));
}

void TimelineWidget::setTotalFrames(int frames, double fps) {
    totalFrames_ = std::max(1, frames);
    fps_         = fps > 0 ? fps : 30.0;
    thumbnails_.clear();
    update();
}

void TimelineWidget::setPhaseFrames(const PhaseFrames& phases) {
    phases_ = phases;
    update();
}

void TimelineWidget::setCurrentFrame(int frameNumber) {
    currentFrame_ = std::clamp(frameNumber, 0, totalFrames_ - 1);
    update();
}

void TimelineWidget::onThumbnailReady(int index, QImage thumbnail) {
    QPixmap pm = QPixmap::fromImage(thumbnail);
    while (thumbnails_.size() <= index) thumbnails_.append(QPixmap{});
    thumbnails_[index] = std::move(pm);
    update();
}

// ── Geometry helpers ──────────────────────────────────────────────────────────

int TimelineWidget::pixelToFrame(int x) const {
    if (width() <= 1 || totalFrames_ <= 1) return 0;
    double ratio = static_cast<double>(x) / (width() - 1);
    return std::clamp(static_cast<int>(ratio * (totalFrames_ - 1)), 0, totalFrames_ - 1);
}

int TimelineWidget::frameToPixel(int frame) const {
    if (totalFrames_ <= 1) return 0;
    return static_cast<int>(static_cast<double>(frame) / (totalFrames_ - 1) * (width() - 1));
}

// ── Mouse ─────────────────────────────────────────────────────────────────────

void TimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        int frame = pixelToFrame(event->pos().x());
        emit seekRequested(frame);
    }
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    if (dragging_ && (event->buttons() & Qt::LeftButton)) {
        int frame = pixelToFrame(event->pos().x());
        emit seekRequested(frame);
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) dragging_ = false;
}

// ── Painting ──────────────────────────────────────────────────────────────────

void TimelineWidget::paintEvent(QPaintEvent* /*event*/) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    const int W = width();
    const int H = height();

    const QRect scrubRect(0, H - kScrubHeight - kScrubMargin,
                          W, kScrubHeight);
    const QRect stripRect(0, 0, W, H - kScrubHeight - kScrubMargin * 2);

    // Background
    p.fillRect(rect(), QColor(18, 18, 24));

    paintThumbnailStrip(p, stripRect);
    paintPhaseMarkers(p, scrubRect);
    paintPlayhead(p, scrubRect);
    paintTimeLabel(p);
}

void TimelineWidget::paintThumbnailStrip(QPainter& p, const QRect& stripRect) {
    if (thumbnails_.isEmpty() || totalFrames_ == 0) {
        p.setPen(QColor(50, 50, 60));
        p.drawText(stripRect, Qt::AlignCenter,
                   QStringLiteral("Thumbnails loading..."));
        return;
    }

    const int thumbW = stripRect.width() / thumbnails_.size();
    for (int i = 0; i < thumbnails_.size(); ++i) {
        if (thumbnails_[i].isNull()) continue;
        QRect dest(stripRect.left() + i * thumbW,
                   stripRect.top(),
                   thumbW, stripRect.height());
        p.drawPixmap(dest, thumbnails_[i]);
    }

    // Faint grid lines between thumbnails
    p.setPen(QColor(0, 0, 0, 80));
    for (int i = 1; i < thumbnails_.size(); ++i) {
        int x = stripRect.left() + i * thumbW;
        p.drawLine(x, stripRect.top(), x, stripRect.bottom());
    }
}

void TimelineWidget::paintPhaseMarkers(QPainter& p, const QRect& scrubRect) {
    // Scrub track background
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(35, 35, 45));
    p.drawRoundedRect(scrubRect, 3, 3);

    // Phase tick marks
    struct PhaseColor { int frame; QColor color; QString label; };
    const QList<PhaseColor> markers = {
        {phases_.legLift,       QColor(0, 200, 100), QStringLiteral("LL")},
        {phases_.footPlant,     QColor(255, 165, 0),  QStringLiteral("FP")},
        {phases_.maxArmCock,    QColor(100, 150, 255), QStringLiteral("AC")},
        {phases_.releasePoint,  QColor(255, 60, 60),   QStringLiteral("R")},
        {phases_.followThrough, QColor(180, 0, 220),   QStringLiteral("FT")},
    };

    for (const auto& m : markers) {
        if (m.frame < 0) continue;
        int x = frameToPixel(m.frame);
        p.setPen(QPen(m.color, 2));
        p.drawLine(x, scrubRect.top(), x, scrubRect.bottom());

        // Small label above tick
        p.setPen(m.color);
        QFont f;
        f.setPointSizeF(7.0);
        f.setBold(true);
        p.setFont(f);
        p.drawText(x - 6, scrubRect.top() - 2, m.label);
    }
}

void TimelineWidget::paintPlayhead(QPainter& p, const QRect& scrubRect) {
    if (totalFrames_ <= 0) return;

    int x = frameToPixel(currentFrame_);

    // Filled progress
    QRect progress(scrubRect.left(), scrubRect.top(),
                   x, scrubRect.height());
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 160, 220, 120));
    p.drawRoundedRect(progress, 3, 3);

    // Playhead line
    p.setPen(QPen(QColor(0, 200, 255), 2));
    p.drawLine(x, scrubRect.top() - 4, x, scrubRect.bottom() + 4);

    // Playhead diamond handle
    const QPointF diamond[4] = {
        {(double)x,     (double)scrubRect.top() - 4},
        {(double)x + 5, (double)scrubRect.top()},
        {(double)x,     (double)scrubRect.top() + 4},
        {(double)x - 5, (double)scrubRect.top()},
    };
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 200, 255));
    p.drawPolygon(diamond, 4);
}

void TimelineWidget::paintTimeLabel(QPainter& p) {
    double seconds = (fps_ > 0) ? currentFrame_ / fps_ : 0;
    int m = static_cast<int>(seconds) / 60;
    int s = static_cast<int>(seconds) % 60;
    int f = currentFrame_;
    QString label = QStringLiteral("  %1:%2  |  Frame %3 / %4")
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'))
        .arg(f)
        .arg(totalFrames_);

    QFont font;
    font.setPointSizeF(8.5);
    p.setFont(font);
    p.setPen(QColor(140, 140, 160));
    p.drawText(rect().adjusted(4, 0, -4, -2),
               Qt::AlignBottom | Qt::AlignLeft, label);
}
