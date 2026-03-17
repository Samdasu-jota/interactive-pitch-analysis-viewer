#pragma once
#include <QWidget>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include "VideoPlayerWidget.h"
#include "TimelineWidget.h"
#include "../models/PitcherProfile.h"

// ComparisonView shows two VideoPlayerWidgets side-by-side:
//   Left:  user's uploaded pitch
//   Right: selected pro pitcher's video
//
// Playback is optionally synchronized — when sync is enabled, seeking
// either player sends a seekRequested signal that both players respond to.
class ComparisonView : public QWidget {
    Q_OBJECT

public:
    explicit ComparisonView(QWidget* parent = nullptr);

    VideoPlayerWidget* userPlayer()       { return userPlayer_; }
    VideoPlayerWidget* comparisonPlayer() { return compPlayer_; }
    TimelineWidget*    userTimeline()     { return userTimeline_; }
    TimelineWidget*    compTimeline()     { return compTimeline_; }

    void setUserLabel(const QString& label);
    void setComparisonPitcher(const PitcherMatch& match);
    void setSyncEnabled(bool enabled);
    bool isSyncEnabled() const { return syncEnabled_; }

signals:
    void seekRequested(int frameNumber);      // synced seek
    void exportComparisonRequested();
    void comparisonVideoDropped(QString path); // user dropped a video onto right panel

public slots:
    void onUserFrameAdvanced(int frameNumber);
    void onComparisonFrameAdvanced(int frameNumber);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QSplitter*         splitter_{nullptr};
    VideoPlayerWidget* userPlayer_{nullptr};
    VideoPlayerWidget* compPlayer_{nullptr};
    TimelineWidget*    userTimeline_{nullptr};
    TimelineWidget*    compTimeline_{nullptr};
    QLabel*            userLabel_{nullptr};
    QLabel*            compLabel_{nullptr};
    QWidget*           deltaPanel_{nullptr};

    bool               syncEnabled_{true};
    bool               blockSync_{false};   // prevent recursive sync signals
};
