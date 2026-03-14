#pragma once
#include <QDockWidget>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHash>
#include "../models/BiomechanicsMetrics.h"
#include "MetricCard.h"

// MetricsPanel is a dock widget on the right side showing all 8 biomechanical
// metrics as animated MetricCard widgets.
class MetricsPanel : public QDockWidget {
    Q_OBJECT

public:
    explicit MetricsPanel(QWidget* parent = nullptr);

public slots:
    void updateMetrics(const BiomechanicsMetrics& metrics);
    void clearMetrics();

private:
    void buildCards();
    void addCard(const QString& key, MetricCard* card);

    QWidget*      container_{nullptr};
    QVBoxLayout*  layout_{nullptr};
    QHash<QString, MetricCard*> cards_;
};
