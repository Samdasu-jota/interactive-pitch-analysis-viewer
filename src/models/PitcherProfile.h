#pragma once
#include <QString>
#include <QUrl>

// A pro pitcher entry from the backend database
struct PitcherProfile {
    QString id;
    QString name;
    QString team;
    QString throwingArm;   // "right" or "left"
    QUrl    photoUrl;
    QUrl    videoUrl;
};

// Top-N similarity match from /analyze/upload response
struct PitcherMatch {
    PitcherProfile profile;
    double similarityScore{0.0};   // 0.0–1.0 cosine similarity
    QStringList similarMechanics;
    QStringList coachingCues;
    QStringList notableDifferences;
};
