"""
Video analysis pipeline using MediaPipe Pose.

analyze_video_bytes(data) → dict matching the /analyze/upload response schema.
"""

import tempfile, os, sys
import cv2
import mediapipe as mp
import numpy as np

from phase_detector import detect_phases
from metrics import compute_metrics

mp_pose = mp.solutions.pose


def analyze_video_bytes(data: bytes) -> dict:
    """
    Run MediaPipe Pose on a video given as raw bytes.
    Returns a dict matching the C++ JsonParser's expected schema.
    """
    # Write to temp file (OpenCV needs a file path)
    suffix = ".mp4"
    with tempfile.NamedTemporaryFile(suffix=suffix, delete=False) as f:
        f.write(data)
        tmp_path = f.name

    try:
        return _analyze_file(tmp_path)
    finally:
        os.unlink(tmp_path)


def _analyze_file(path: str) -> dict:
    cap = cv2.VideoCapture(path)
    if not cap.isOpened():
        raise RuntimeError(f"Cannot open video: {path}")

    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    fps          = cap.get(cv2.CAP_PROP_FPS) or 30.0
    width        = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height       = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))

    pose_results   = []   # mediapipe result per frame
    all_landmarks  = []   # just the landmark lists (for phase/metric calc)
    pose_frames    = []   # JSON-serialisable pose frames for response

    with mp_pose.Pose(
        static_image_mode=False,
        model_complexity=1,
        smooth_landmarks=True,
        min_detection_confidence=0.5,
        min_tracking_confidence=0.5,
    ) as pose:
        frame_idx = 0
        while True:
            ret, bgr = cap.read()
            if not ret:
                break

            rgb    = cv2.cvtColor(bgr, cv2.COLOR_BGR2RGB)
            result = pose.process(rgb)

            if result.pose_landmarks:
                lms = result.pose_landmarks.landmark
                all_landmarks.append(lms)
                pose_frames.append({
                    "frame_number": frame_idx,
                    "frame_shape":  [height, width],
                    "landmarks": [
                        {
                            "x":          float(lm.x),
                            "y":          float(lm.y),
                            "z":          float(lm.z),
                            "visibility": float(lm.visibility),
                        }
                        for lm in lms
                    ],
                })

            frame_idx += 1

    cap.release()

    if not all_landmarks:
        raise RuntimeError("MediaPipe detected no poses in this video.")

    # Map each detected pose frame back to its original frame index
    detected_frame_indices = [pf["frame_number"] for pf in pose_frames]

    # Phase detection operates on detected frames only
    phases = detect_phases(all_landmarks, fps)

    # Map internal indices → original frame indices
    def _remap(idx):
        if idx < 0 or idx >= len(detected_frame_indices):
            return -1
        return detected_frame_indices[idx]

    phases_mapped = {k: _remap(v) for k, v in phases.items()}
    release_internal = phases.get("release_point", len(all_landmarks) // 2)

    # Compute biomechanics metrics
    features = compute_metrics(all_landmarks, release_internal)
    features["release_frame_index"] = _remap(release_internal)

    return {
        "total_frames":    total_frames,
        "detected_frames": len(pose_frames),
        "poses":           pose_frames,
        "features":        features,
        "phases":          phases_mapped,
        "recommendations": _mock_recommendations(),
    }


def _mock_recommendations():
    """
    Pitcher similarity matching would require a full biomechanics database.
    Return curated coaching cues based on common RHP mechanics for now.
    """
    return [
        {
            "pitcher_name": "Gerrit Cole",
            "throws": "right",
            "similarity_score": "87.5%",
            "summary": "Strong mechanical similarity. Focus on earlier hip engagement.",
            "similar_mechanics": [
                "Elbow angle at release",
                "Release height",
                "Stride length",
            ],
            "coaching_cues": [
                "Initiate hip rotation 8–10% earlier relative to foot plant",
                "Maintain 90° shoulder abduction through acceleration phase",
                "Drive back knee toward plate to maximize hip extension",
            ],
            "notable_differences": [
                "Hip-shoulder separation slightly below Cole's average",
                "Arm extension can be increased at release",
            ],
        },
        {
            "pitcher_name": "Clayton Kershaw",
            "throws": "left",
            "similarity_score": "79.2%",
            "summary": "Efficient arm path and stride mechanics.",
            "similar_mechanics": [
                "Delivery tempo",
                "Arm extension",
                "Knee flexion at foot plant",
            ],
            "coaching_cues": [
                "Delay shoulder rotation slightly to increase hip-shoulder separation",
                "Steepen arm slot for more downward plane",
            ],
            "notable_differences": [
                "Shoulder rotation lags Kershaw's peak value",
                "Stride direction slightly more closed",
            ],
        },
        {
            "pitcher_name": "Yu Darvish",
            "throws": "right",
            "similarity_score": "71.8%",
            "summary": "Good rotational power. Arm extension and follow-through can improve.",
            "similar_mechanics": [
                "Hip rotation angle at release",
                "Stride length",
            ],
            "coaching_cues": [
                "Extend arm further through the release zone",
                "Allow trunk to flex more aggressively post-release",
            ],
            "notable_differences": [
                "Arm extension below Darvish's average",
                "Follow-through deceleration starts earlier",
            ],
        },
    ]
