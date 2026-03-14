#pragma once
#include <array>
#include <utility>

// Full MediaPipe Pose 33-keypoint skeleton topology.
// Each pair is (landmarkA, landmarkB) defining a bone segment to draw.
// Reference: https://developers.google.com/mediapipe/solutions/vision/pose_landmarker
namespace SkeletonTopology {

    constexpr std::array<std::pair<int, int>, 35> kConnections = {{
        // Face
        {0, 1},  {1, 2},  {2, 3},  {3, 7},
        {0, 4},  {4, 5},  {5, 6},  {6, 8},

        // Torso
        {11, 12},  // shoulders
        {11, 23},  // left shoulder → left hip
        {12, 24},  // right shoulder → right hip
        {23, 24},  // hips

        // Left arm
        {11, 13},  // left shoulder → left elbow
        {13, 15},  // left elbow → left wrist

        // Right arm
        {12, 14},  // right shoulder → right elbow
        {14, 16},  // right elbow → right wrist

        // Left hand (wrist → fingers)
        {15, 17}, {15, 19}, {15, 21},

        // Right hand
        {16, 18}, {16, 20}, {16, 22},

        // Left leg
        {23, 25},  // left hip → left knee
        {25, 27},  // left knee → left ankle
        {27, 29},  // left ankle → left heel
        {29, 31},  // left heel → left foot index

        // Right leg
        {24, 26},  // right hip → right knee
        {26, 28},  // right knee → right ankle
        {28, 30},  // right ankle → right heel
        {30, 32},  // right heel → right foot index

        // Feet cross
        {27, 31},
        {28, 32},
    }};

    // Body part groups for color-coded rendering
    constexpr std::array<std::pair<int, int>, 4> kArmConnections = {{
        {11, 13}, {13, 15}, {12, 14}, {14, 16}
    }};

    constexpr std::array<std::pair<int, int>, 4> kLegConnections = {{
        {23, 25}, {25, 27}, {24, 26}, {26, 28}
    }};

    constexpr std::array<std::pair<int, int>, 3> kTorsoConnections = {{
        {11, 12}, {11, 23}, {12, 24}
    }};

} // namespace SkeletonTopology
