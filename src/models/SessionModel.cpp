#include "SessionModel.h"

SessionModel::SessionModel(QObject* parent)
    : QObject(parent) {}

const PoseFrame* SessionModel::poseFrameAt(int frameNumber) const {
    if (!analysisResult_.valid) return nullptr;
    return analysisResult_.poseAt(frameNumber);
}

void SessionModel::setVideoMetadata(const QString& path, int totalFrames, double fps) {
    videoPath_    = path;
    totalFrames_  = totalFrames;
    fps_          = fps;
    currentFrame_.store(0);
    emit videoLoaded(path, totalFrames, fps);
}

void SessionModel::setAnalysisResult(const AnalysisResult& result) {
    analysisResult_ = result;
    emit analysisResultReady(result);
}

void SessionModel::setCurrentFrame(int frameNumber) {
    currentFrame_.store(frameNumber);
    emit currentFrameChanged(frameNumber);
}

void SessionModel::setPlaying(bool playing) {
    playing_.store(playing);
    emit playbackStateChanged(playing);
}

void SessionModel::setLoopEnabled(bool enabled) {
    loopEnabled_ = enabled;
}

void SessionModel::setPoseVisible(bool visible) {
    poseVisible_ = visible;
    emit poseVisibilityChanged(visible);
}

void SessionModel::setComparisonPitcher(const QString& pitcherId) {
    comparisonPitcherId_ = pitcherId;
    emit comparisonPitcherChanged(pitcherId);
}

void SessionModel::clearAnalysis() {
    analysisResult_ = {};
    comparisonPitcherId_.clear();
    emit analysisCleared();
}
