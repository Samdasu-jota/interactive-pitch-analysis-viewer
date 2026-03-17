#pragma once
#include <QString>
#include <QColor>
#include <unordered_map>

// Per-metric threshold ranges for the gauge coloring in MetricCard.
// Values inside [optimalMin, optimalMax] are green.
// Values inside [warnMin, warnMax] are yellow.
// Values outside are red.
struct MetricRange {
    double optimalMin;
    double optimalMax;
    double warnMin;
    double warnMax;
    double displayMin;   // gauge scale minimum
    double displayMax;   // gauge scale maximum
    QString unit;        // display unit string

    // Returns the color for a given value
    QColor colorFor(double value) const {
        if (value >= optimalMin && value <= optimalMax) return QColor(0x4C, 0xAF, 0x50);  // green
        if (value >= warnMin    && value <= warnMax)    return QColor(0xFF, 0xC1, 0x07);  // amber
        return QColor(0xF4, 0x43, 0x36);  // red
    }

    // Normalize value to [0, 1] for gauge bar width
    double normalize(double value) const {
        if (displayMax <= displayMin) return 0.0;
        double clamped = qBound(displayMin, value, displayMax);
        return (clamped - displayMin) / (displayMax - displayMin);
    }
};

// Default thresholds based on baseball biomechanics literature.
// All angles in degrees, all distances normalized unless noted.
namespace MetricThresholds {

    inline const std::unordered_map<QString, MetricRange>& defaults() {
        static const std::unordered_map<QString, MetricRange> kThresholds = {
            // key                      optMin  optMax  warnMin  warnMax  dispMin  dispMax  unit
            {"elbowAngle",            {  85.0,  100.0,  75.0,  110.0,   30.0,   170.0,  "°"   }},
            {"hipShoulderSeparation", {  45.0,   65.0,  30.0,   75.0,    0.0,    90.0,  "°"   }},
            {"shoulderRotationAngle", {  80.0,  110.0,  60.0,  130.0,    0.0,   180.0,  "°"   }},
            {"strideLength",          {  0.75,   0.95,  0.60,   1.05,   0.0,     1.2,   "×ht" }},
            {"deliveryTempo",         {  0.55,   0.70,  0.45,   0.80,   0.0,     1.0,   ""    }},
            {"releaseHeight",         {  0.30,   0.55,  0.25,   0.65,   0.0,     1.0,   ""    }},
            {"armExtension",          {  0.85,   1.00,  0.70,   1.00,   0.0,     1.0,   ""    }},
            {"leadKneeFlexion",       {  30.0,   55.0,  20.0,   65.0,    0.0,    90.0,  "°"   }},
            {"armSlotHeight",         {  0.55,   0.80,  0.40,   0.90,    0.0,     1.0,  ""    }},
            {"hipRotationAngle",      {  60.0,   85.0,  45.0,   90.0,    0.0,    90.0,  "°"   }},
            {"strideDirectionAngle",  {   0.0,   15.0,   0.0,   25.0,    0.0,    45.0,  "°"   }},
            {"releaseLateralPosition",{ -0.20,   0.20, -0.40,   0.40,   -1.0,     1.0,  ""    }},
            {"releaseExtension",      {  0.70,   1.20,  0.50,   1.50,    0.0,     1.5,  "×ht" }},
        };
        return kThresholds;
    }

    inline MetricRange getRange(const QString& metricKey) {
        const auto& m = defaults();
        auto it = m.find(metricKey);
        if (it != m.end()) return it->second;
        // Default passthrough range
        return {0.0, 100.0, 0.0, 100.0, 0.0, 100.0, ""};
    }

} // namespace MetricThresholds
