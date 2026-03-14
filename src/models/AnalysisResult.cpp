#include "AnalysisResult.h"
#include <algorithm>

const PoseFrame* AnalysisResult::poseAt(int frameNumber) const {
    // poseFrames is ordered by frameNumber; use binary search for O(log n)
    auto it = std::lower_bound(
        poseFrames.begin(), poseFrames.end(), frameNumber,
        [](const PoseFrame& pf, int n) { return pf.frameNumber < n; });

    if (it != poseFrames.end() && it->frameNumber == frameNumber) {
        return &(*it);
    }
    return nullptr;
}

PitchPhase PhaseFrames::phaseAtFrame(int frame) const {
    if (frame < 0) return PitchPhase::Unknown;

    if (releasePoint >= 0 && frame >= followThrough)    return PitchPhase::FollowThrough;
    if (releasePoint >= 0 && frame >= releasePoint)     return PitchPhase::Release;
    if (maxArmCock   >= 0 && frame >= maxArmCock)       return PitchPhase::Acceleration;
    if (footPlant    >= 0 && frame >= footPlant)        return PitchPhase::ArmCock;
    if (legLift      >= 0 && frame >= legLift)          return PitchPhase::Stride;
    if (legLift      >= 0 && frame < legLift)           return PitchPhase::Windup;

    return PitchPhase::Windup;
}
