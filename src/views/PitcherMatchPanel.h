#pragma once
#include <QDockWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <vector>
#include "../models/PitcherProfile.h"

// Shows the top-3 pitcher similarity matches from the analysis response.
// Each entry has: pitcher name, throwing arm, similarity %, compare button.
class PitcherMatchPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit PitcherMatchPanel(QWidget* parent = nullptr);

public slots:
    void setMatches(const std::vector<PitcherMatch>& matches);
    void clearMatches();

signals:
    void compareRequested(const QString& pitcherId);

private:
    QWidget*     container_{nullptr};
    QVBoxLayout* layout_{nullptr};
};
