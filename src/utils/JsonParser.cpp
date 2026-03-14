#include "JsonParser.h"
#include <nlohmann/json.hpp>
#include <QDebug>

using json = nlohmann::json;

// ── Helpers ──────────────────────────────────────────────────────────────────

static double safeDouble(const json& obj, const std::string& key, double def = 0.0) {
    try {
        if (obj.contains(key) && !obj[key].is_null()) return obj[key].get<double>();
    } catch (...) {}
    return def;
}

static int safeInt(const json& obj, const std::string& key, int def = -1) {
    try {
        if (obj.contains(key) && !obj[key].is_null()) return obj[key].get<int>();
    } catch (...) {}
    return def;
}

static QString safeString(const json& obj, const std::string& key, const QString& def = {}) {
    try {
        if (obj.contains(key) && obj[key].is_string())
            return QString::fromStdString(obj[key].get<std::string>());
    } catch (...) {}
    return def;
}

// ── PoseFrame ────────────────────────────────────────────────────────────────

PoseFrame JsonParser::parsePoseFrame(const void* frameObjPtr) {
    const json& obj = *static_cast<const json*>(frameObjPtr);
    PoseFrame frame;

    frame.frameNumber = safeInt(obj, "frame_number");

    if (obj.contains("frame_shape") && obj["frame_shape"].is_array()
        && obj["frame_shape"].size() >= 2) {
        frame.sourceHeight = obj["frame_shape"][0].get<int>();
        frame.sourceWidth  = obj["frame_shape"][1].get<int>();
    }

    if (obj.contains("landmarks") && obj["landmarks"].is_array()) {
        const auto& lms = obj["landmarks"];
        int count = std::min<int>(static_cast<int>(lms.size()), 33);
        for (int i = 0; i < count; ++i) {
            const auto& lm = lms[i];
            frame.landmarks[i].x          = safeDouble(lm, "x");
            frame.landmarks[i].y          = safeDouble(lm, "y");
            frame.landmarks[i].z          = safeDouble(lm, "z");
            frame.landmarks[i].visibility = safeDouble(lm, "visibility");
        }
    }

    return frame;
}

// ── BiomechanicsMetrics ───────────────────────────────────────────────────────

BiomechanicsMetrics JsonParser::parseMetrics(const void* metricsObjPtr) {
    const json& obj = *static_cast<const json*>(metricsObjPtr);
    BiomechanicsMetrics m;

    m.elbowAngle              = safeDouble(obj, "elbow_angle");
    m.armSlotHeight           = safeDouble(obj, "arm_slot_height");
    m.armExtension            = safeDouble(obj, "arm_extension_distance");
    m.releaseHeight           = safeDouble(obj, "release_height");
    m.hipShoulderSeparation   = safeDouble(obj, "hip_shoulder_separation");
    m.shoulderRotationAngle   = safeDouble(obj, "shoulder_rotation_angle");
    m.hipRotationAngle        = safeDouble(obj, "hip_rotation_angle");
    m.releaseExtension        = safeDouble(obj, "release_extension");
    m.strideLength            = safeDouble(obj, "stride_length");
    m.strideDirectionAngle    = safeDouble(obj, "stride_direction_angle");
    m.leadKneeFlexion         = safeDouble(obj, "knee_flexion");
    m.releaseLateralPosition  = safeDouble(obj, "release_lateral_position");
    m.deliveryTempo           = safeDouble(obj, "delivery_tempo");
    m.releaseFrame            = safeInt(obj, "release_frame_index");

    return m;
}

// ── PhaseFrames ───────────────────────────────────────────────────────────────

PhaseFrames JsonParser::parsePhaseFrames(const void* phasesObjPtr) {
    const json& obj = *static_cast<const json*>(phasesObjPtr);
    PhaseFrames pf;

    pf.legLift       = safeInt(obj, "leg_lift",       -1);
    pf.footPlant     = safeInt(obj, "foot_plant",     -1);
    pf.maxArmCock    = safeInt(obj, "max_arm_cock",   -1);
    pf.releasePoint  = safeInt(obj, "release_point",  -1);
    pf.followThrough = safeInt(obj, "follow_through", -1);

    return pf;
}

// ── PitcherProfile ────────────────────────────────────────────────────────────

PitcherProfile JsonParser::parsePitcherProfile(const void* profileObjPtr) {
    const json& obj = *static_cast<const json*>(profileObjPtr);
    PitcherProfile p;

    p.id          = safeString(obj, "id");
    p.name        = safeString(obj, "name");
    p.team        = safeString(obj, "team");
    p.throwingArm = safeString(obj, "throws");

    if (obj.contains("video_url") && obj["video_url"].is_string())
        p.videoUrl = QUrl(QString::fromStdString(obj["video_url"].get<std::string>()));

    return p;
}

// ── PitcherMatch ──────────────────────────────────────────────────────────────

PitcherMatch JsonParser::parsePitcherMatch(const void* matchObjPtr) {
    const json& obj = *static_cast<const json*>(matchObjPtr);
    PitcherMatch m;

    // The similarity score may come as a string (e.g. "87.5%") or a float
    if (obj.contains("similarity_score")) {
        if (obj["similarity_score"].is_number()) {
            m.similarityScore = obj["similarity_score"].get<double>();
            if (m.similarityScore > 1.0) m.similarityScore /= 100.0;  // normalize %
        } else if (obj["similarity_score"].is_string()) {
            std::string s = obj["similarity_score"].get<std::string>();
            if (!s.empty() && s.back() == '%') s.pop_back();
            try { m.similarityScore = std::stod(s) / 100.0; } catch (...) {}
        }
    }

    m.profile.name = safeString(obj, "pitcher_name");
    m.profile.throwingArm = safeString(obj, "throws");
    m.profile.id = QString::fromStdString(
        m.profile.name.toLower().replace(' ', '_').toStdString());

    if (obj.contains("similar_mechanics") && obj["similar_mechanics"].is_array()) {
        for (const auto& item : obj["similar_mechanics"])
            if (item.is_string())
                m.similarMechanics << QString::fromStdString(item.get<std::string>());
    }

    if (obj.contains("coaching_cues") && obj["coaching_cues"].is_array()) {
        for (const auto& item : obj["coaching_cues"])
            if (item.is_string())
                m.coachingCues << QString::fromStdString(item.get<std::string>());
    }

    if (obj.contains("notable_differences") && obj["notable_differences"].is_array()) {
        for (const auto& item : obj["notable_differences"])
            if (item.is_string())
                m.notableDifferences << QString::fromStdString(item.get<std::string>());
    }

    return m;
}

// ── Public API ────────────────────────────────────────────────────────────────

AnalysisResult JsonParser::parseAnalysisResponse(const QByteArray& jsonData) {
    AnalysisResult result;

    try {
        json root = json::parse(jsonData.constData(), jsonData.constData() + jsonData.size());

        // Pose frames
        if (root.contains("poses") && root["poses"].is_array()) {
            result.poseFrames.reserve(root["poses"].size());
            for (const auto& frameObj : root["poses"]) {
                result.poseFrames.push_back(parsePoseFrame(&frameObj));
            }
            result.totalDetectedFrames = safeInt(root, "detected_frames",
                                                 static_cast<int>(result.poseFrames.size()));
        }

        // Metrics — may be nested under "features" or "metrics"
        if (root.contains("features") && root["features"].is_object()) {
            result.metrics = parseMetrics(&root["features"]);
        } else if (root.contains("metrics") && root["metrics"].is_object()) {
            result.metrics = parseMetrics(&root["metrics"]);
        }

        // Phase frames
        if (root.contains("phases") && root["phases"].is_object()) {
            result.phases = parsePhaseFrames(&root["phases"]);
        } else {
            // Fall back: use release frame from metrics
            result.phases.releasePoint = result.metrics.releaseFrame;
        }

        // Top-3 recommendations
        if (root.contains("recommendations") && root["recommendations"].is_array()) {
            for (const auto& matchObj : root["recommendations"]) {
                result.topMatches.push_back(parsePitcherMatch(&matchObj));
            }
        }

        // Coaching text
        if (root.contains("summary") && root["summary"].is_string()) {
            result.recommendations << QString::fromStdString(root["summary"].get<std::string>());
        }

        result.valid = !result.poseFrames.empty() || result.metrics.isValid();

    } catch (const json::exception& e) {
        qWarning() << "JsonParser: failed to parse analysis response:" << e.what();
        result.valid = false;
    }

    return result;
}

std::vector<PitcherProfile> JsonParser::parsePitcherList(const QByteArray& jsonData) {
    std::vector<PitcherProfile> profiles;
    try {
        json root = json::parse(jsonData.constData(), jsonData.constData() + jsonData.size());
        const json* pitchers = nullptr;

        if (root.contains("pitchers") && root["pitchers"].is_array())
            pitchers = &root["pitchers"];
        else if (root.is_array())
            pitchers = &root;

        if (pitchers) {
            for (const auto& obj : *pitchers)
                profiles.push_back(parsePitcherProfile(&obj));
        }
    } catch (const json::exception& e) {
        qWarning() << "JsonParser: failed to parse pitcher list:" << e.what();
    }
    return profiles;
}

PitcherProfile JsonParser::parsePitcherDetails(const QByteArray& jsonData) {
    try {
        json root = json::parse(jsonData.constData(), jsonData.constData() + jsonData.size());
        return parsePitcherProfile(&root);
    } catch (const json::exception& e) {
        qWarning() << "JsonParser: failed to parse pitcher details:" << e.what();
    }
    return {};
}
