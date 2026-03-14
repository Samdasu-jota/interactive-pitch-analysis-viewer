#pragma once

// A single MediaPipe Pose landmark.
// Coordinates are normalized to [0.0, 1.0] relative to the frame dimensions.
// z is an approximate depth relative to the hip midpoint (negative = in front).
// visibility is a confidence score in [0.0, 1.0].
struct PoseLandmark {
    float x{0.f};
    float y{0.f};
    float z{0.f};
    float visibility{0.f};
};
