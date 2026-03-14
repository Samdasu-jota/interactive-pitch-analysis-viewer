#pragma once
#include <QString>

// The 14 biomechanical metrics computed by the Python AI backend.
// All angles are in degrees. Distances are normalized (0.0–1.0) relative to
// estimated body height. Ratios are in [0.0, 1.0].
struct BiomechanicsMetrics {
    // Arm mechanics
    double elbowAngle{0.0};             // shoulder-elbow-wrist angle at release (°)
    double armSlotHeight{0.0};          // vertical hand position (normalized)
    double armExtension{0.0};           // forward reach from shoulder (0.0–1.0)
    double releaseHeight{0.0};          // vertical coord at release point (normalized)

    // Rotational
    double hipShoulderSeparation{0.0};  // angular diff between hip/shoulder rotation (°)
    double shoulderRotationAngle{0.0};  // shoulder rotation at release (°)
    double hipRotationAngle{0.0};       // hip rotation at release (°)
    double releaseExtension{0.0};       // forward distance from shoulder (normalized)

    // Stride
    double strideLength{0.0};           // ankle-to-ankle distance (normalized)
    double strideDirectionAngle{0.0};   // stride angle toward plate (°)
    double leadKneeFlexion{0.0};        // lead knee bend at foot plant (°)
    double releaseLateralPosition{0.0}; // side distance at release (normalized)

    // Temporal
    double deliveryTempo{0.0};          // release timing as fraction of delivery (0–1)
    int    releaseFrame{-1};            // frame index of ball release

    bool isValid() const { return releaseFrame >= 0; }
};

// Pitching delivery phases — used for timeline markers and overlay labels
enum class PitchPhase {
    Unknown,
    Windup,
    LegLift,
    Stride,
    ArmCock,
    Acceleration,
    Release,
    FollowThrough
};

inline QString phaseToString(PitchPhase phase) {
    switch (phase) {
    case PitchPhase::Windup:        return QStringLiteral("WINDUP");
    case PitchPhase::LegLift:       return QStringLiteral("LEG LIFT");
    case PitchPhase::Stride:        return QStringLiteral("STRIDE");
    case PitchPhase::ArmCock:       return QStringLiteral("ARM COCK");
    case PitchPhase::Acceleration:  return QStringLiteral("ACCELERATION");
    case PitchPhase::Release:       return QStringLiteral("RELEASE");
    case PitchPhase::FollowThrough: return QStringLiteral("FOLLOW THROUGH");
    default:                        return QStringLiteral("—");
    }
}

// Key event frame indices within a pitching delivery
struct PhaseFrames {
    int legLift{-1};
    int footPlant{-1};
    int maxArmCock{-1};
    int releasePoint{-1};
    int followThrough{-1};

    PitchPhase phaseAtFrame(int frame) const;
};
