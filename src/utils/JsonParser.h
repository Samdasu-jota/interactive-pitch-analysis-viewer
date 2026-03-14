#pragma once
#include <QByteArray>
#include <QString>
#include "../models/AnalysisResult.h"
#include "../models/PitcherProfile.h"
#include <vector>

// Parses JSON responses from the Python FastAPI backend into C++ model objects.
// All methods are static — no state.
class JsonParser {
public:
    // Parse the full /analyze/upload response into an AnalysisResult.
    // Returns an AnalysisResult with valid=false on parse failure.
    static AnalysisResult parseAnalysisResponse(const QByteArray& json);

    // Parse the /database/pitchers list response.
    static std::vector<PitcherProfile> parsePitcherList(const QByteArray& json);

    // Parse a single /database/pitcher/{id} response.
    static PitcherProfile parsePitcherDetails(const QByteArray& json);

private:
    static PoseFrame parsePoseFrame(const void* frameObj);
    static BiomechanicsMetrics parseMetrics(const void* metricsObj);
    static PhaseFrames parsePhaseFrames(const void* phasesObj);
    static PitcherMatch parsePitcherMatch(const void* matchObj);
    static PitcherProfile parsePitcherProfile(const void* profileObj);
};
