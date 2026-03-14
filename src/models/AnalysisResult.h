#pragma once
#include "PoseFrame.h"
#include "BiomechanicsMetrics.h"
#include "PitcherProfile.h"
#include <vector>
#include <QStringList>

// The complete result returned by the AI backend for one uploaded pitch video.
// Populated by JsonParser and stored in SessionModel.
struct AnalysisResult {
    std::vector<PoseFrame>   poseFrames;    // one per video frame (may be sparse)
    BiomechanicsMetrics      metrics;
    PhaseFrames              phases;
    std::vector<PitcherMatch> topMatches;   // top-3 similarity matches
    QStringList              recommendations;
    bool                     valid{false};

    // Look up pose frame by frame number. Returns nullptr if not found.
    const PoseFrame* poseAt(int frameNumber) const;

    // Frame count reported by the backend (may exceed poseFrames.size() if
    // some frames had no detectable pose)
    int totalDetectedFrames{0};
};
