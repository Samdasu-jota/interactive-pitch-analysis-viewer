#pragma once
#include "PoseLandmark.h"
#include <array>

// MediaPipe Pose produces 33 landmarks per frame.
// See: https://developers.google.com/mediapipe/solutions/vision/pose_landmarker
struct PoseFrame {
    int frameNumber{-1};
    std::array<PoseLandmark, 33> landmarks{};
    int sourceWidth{640};
    int sourceHeight{480};

    bool isValid() const { return frameNumber >= 0; }

    // Convenience: return landmark only if visibility passes threshold
    const PoseLandmark* landmarkIfVisible(int index, float minVisibility = 0.5f) const {
        if (index < 0 || index >= 33) return nullptr;
        return (landmarks[index].visibility >= minVisibility) ? &landmarks[index] : nullptr;
    }
};

// MediaPipe landmark indices (subset used for pitching analysis)
namespace LandmarkIndex {
    constexpr int LEFT_SHOULDER  = 11;
    constexpr int RIGHT_SHOULDER = 12;
    constexpr int LEFT_ELBOW     = 13;
    constexpr int RIGHT_ELBOW    = 14;
    constexpr int LEFT_WRIST     = 15;
    constexpr int RIGHT_WRIST    = 16;
    constexpr int LEFT_HIP       = 23;
    constexpr int RIGHT_HIP      = 24;
    constexpr int LEFT_KNEE      = 25;
    constexpr int RIGHT_KNEE     = 26;
    constexpr int LEFT_ANKLE     = 27;
    constexpr int RIGHT_ANKLE    = 28;
    constexpr int NOSE           = 0;
}
