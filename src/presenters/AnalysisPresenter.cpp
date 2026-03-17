#include "AnalysisPresenter.h"
#include <QDebug>

AnalysisPresenter::AnalysisPresenter(SessionModel*      model,
                                      MetricsPanel*      metricsPanel,
                                      TimelineWidget*    timeline,
                                      ApiClient*         apiClient,
                                      QObject*           parent)
    : QObject(parent)
    , model_(model)
    , metricsPanel_(metricsPanel)
    , timeline_(timeline)
    , apiClient_(apiClient)
    , apiBaseUrl_(apiClient_->baseUrl())
{
    // Real-time per-frame metric updates during playback
    connect(model_, &SessionModel::currentFrameChanged,
            this, &AnalysisPresenter::onFrameChanged);
}

void AnalysisPresenter::startAnalysis(const QString& videoPath) {
    if (analysisRunning_) {
        qWarning() << "AnalysisPresenter: analysis already in progress";
        return;
    }

    if (videoPath.isEmpty()) {
        emit analysisError(QStringLiteral("No video loaded. Open a video first."));
        return;
    }

    analysisRunning_ = true;
    emit analysisStarted();

    // Spawn a one-shot analysis thread.
    // We use Qt::AutoDeleteOnDestruction pattern — connect finished to deleteLater.
    analysisThread_ = new AnalysisThread(videoPath, apiBaseUrl_, this);

    connect(analysisThread_, &AnalysisThread::analysisComplete,
            this, &AnalysisPresenter::onAnalysisComplete,
            Qt::QueuedConnection);

    connect(analysisThread_, &AnalysisThread::analysisError,
            this, &AnalysisPresenter::onAnalysisError,
            Qt::QueuedConnection);

    connect(analysisThread_, &AnalysisThread::uploadProgress,
            this, &AnalysisPresenter::onUploadProgress,
            Qt::QueuedConnection);

    connect(analysisThread_, &AnalysisThread::finished,
            analysisThread_, &QObject::deleteLater);

    analysisThread_->start();
    qDebug() << "AnalysisPresenter: analysis thread started";
}

void AnalysisPresenter::onAnalysisComplete(AnalysisResult result) {
    analysisRunning_ = false;
    analysisThread_  = nullptr;

    qDebug() << "AnalysisPresenter: analysis complete —"
             << result.poseFrames.size() << "frames,"
             << result.topMatches.size() << "matches";

    // Write to session model (UI thread — safe)
    model_->setAnalysisResult(result);

    // Update UI panels
    metricsPanel_->updateMetrics(result.metrics);
    timeline_->setPhaseFrames(result.phases);

    emit analysisFinished();
}

void AnalysisPresenter::onAnalysisError(QString message) {
    analysisRunning_ = false;
    analysisThread_  = nullptr;
    qWarning() << "AnalysisPresenter: error —" << message;
    emit analysisError(message);
}

void AnalysisPresenter::onUploadProgress(int percent) {
    emit analysisProgressChanged(percent);
}

void AnalysisPresenter::onFrameChanged(int frameNumber) {
    if (!model_->hasAnalysis()) return;
    const PoseFrame* pose = model_->poseFrameAt(frameNumber);
    if (!pose || !pose->isValid()) return;
    metricsPanel_->updateMetrics(
        computeFrameMetrics(*pose, model_->analysisResult().metrics));
}

BiomechanicsMetrics AnalysisPresenter::computeFrameMetrics(
    const PoseFrame& pose, const BiomechanicsMetrics& fallback)
{
    auto lm = [&](int i) -> const PoseLandmark& { return pose.landmarks[i]; };

    auto dist2d = [](float x1, float y1, float x2, float y2) {
        return std::hypot(x2 - x1, y2 - y1);
    };

    // Angle at vertex (vx,vy) between points A and B
    auto angle3 = [](float ax, float ay, float vx, float vy, float bx, float by) -> float {
        float vax = ax - vx, vay = ay - vy;
        float vbx = bx - vx, vby = by - vy;
        float dot = vax * vbx + vay * vby;
        float mag = std::hypot(vax, vay) * std::hypot(vbx, vby);
        if (mag < 1e-9f) return 0.0f;
        return std::acos(std::clamp(dot / mag, -1.0f, 1.0f)) * 180.0f / static_cast<float>(M_PI);
    };

    // Body height: nose to mid-ankle (MediaPipe normalized coords)
    float mid_ax = (lm(27).x + lm(28).x) * 0.5f;
    float mid_ay = (lm(27).y + lm(28).y) * 0.5f;
    float body_h = dist2d(lm(0).x, lm(0).y, mid_ax, mid_ay);
    if (body_h < 0.05f) body_h = 0.5f;

    // ── Arm mechanics (right-hand pitcher: landmarks 12,14,16) ────────────────
    float elbow_angle    = angle3(lm(12).x, lm(12).y, lm(14).x, lm(14).y, lm(16).x, lm(16).y);
    float arm_slot       = std::max(0.0f, 1.0f - lm(16).y);   // invert y: higher = larger
    float arm_ext        = dist2d(lm(12).x, lm(12).y, lm(16).x, lm(16).y) / body_h;
    float rel_ext        = std::abs(lm(12).x - lm(16).x) / body_h;

    // ── Rotational mechanics ──────────────────────────────────────────────────
    float sh_dx = lm(12).x - lm(11).x, sh_dy = lm(12).y - lm(11).y;
    float shoulder_rot   = std::abs(std::atan2(sh_dy, std::max(sh_dx, 1e-9f))
                                    * 180.0f / static_cast<float>(M_PI));
    float hi_dx = lm(24).x - lm(23).x, hi_dy = lm(24).y - lm(23).y;
    float hip_rot        = std::abs(std::atan2(hi_dy, std::max(hi_dx, 1e-9f))
                                    * 180.0f / static_cast<float>(M_PI));
    float hip_sh_sep     = std::abs(shoulder_rot - hip_rot);

    // ── Lateral position ──────────────────────────────────────────────────────
    float sh_cx          = (lm(11).x + lm(12).x) * 0.5f;
    float sh_w           = dist2d(lm(11).x, lm(11).y, lm(12).x, lm(12).y);
    float lateral_pos    = (lm(16).x - sh_cx) / std::max(sh_w, 0.01f);

    // ── Stride (single-frame approximation) ───────────────────────────────────
    float stride_len     = std::min(dist2d(lm(27).x, lm(27).y, lm(28).x, lm(28).y) / body_h, 1.5f);
    float st_dx          = lm(27).x - lm(28).x, st_dy = lm(27).y - lm(28).y;
    float stride_dir     = std::abs(std::atan2(st_dy, std::max(std::abs(st_dx), 1e-9f))
                                    * 180.0f / static_cast<float>(M_PI));
    float knee_angle     = angle3(lm(23).x, lm(23).y, lm(25).x, lm(25).y, lm(27).x, lm(27).y);
    float lead_knee      = std::max(0.0f, 180.0f - knee_angle);

    BiomechanicsMetrics m;
    m.elbowAngle             = std::round(elbow_angle * 100.0) / 100.0;
    m.armSlotHeight          = arm_slot;
    m.armExtension           = std::min(static_cast<double>(arm_ext),  1.5);
    m.releaseHeight          = arm_slot;
    m.releaseExtension       = std::min(static_cast<double>(rel_ext),  1.5);
    m.shoulderRotationAngle  = shoulder_rot;
    m.hipRotationAngle       = hip_rot;
    m.hipShoulderSeparation  = hip_sh_sep;
    m.releaseLateralPosition = lateral_pos;
    m.strideLength           = stride_len;
    m.strideDirectionAngle   = stride_dir;
    m.leadKneeFlexion        = lead_knee;
    m.deliveryTempo          = fallback.deliveryTempo;  // release-frame value
    m.releaseFrame           = fallback.releaseFrame;
    return m;
}
