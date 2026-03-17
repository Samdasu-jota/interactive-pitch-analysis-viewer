"""
Compute 14 biomechanical metrics from MediaPipe landmark sequences.
All metrics match the field names expected by JsonParser.cpp (features object).
"""

import math
import numpy as np
from typing import List

# MediaPipe Pose landmark indices
NOSE           = 0
LEFT_SHOULDER  = 11
RIGHT_SHOULDER = 12
LEFT_ELBOW     = 13
RIGHT_ELBOW    = 14
LEFT_WRIST     = 15
RIGHT_WRIST    = 16
LEFT_HIP       = 23
RIGHT_HIP      = 24
LEFT_KNEE      = 25
RIGHT_KNEE     = 26
LEFT_ANKLE     = 27
RIGHT_ANKLE    = 28


def _xy(frame_landmarks, idx):
    lm = frame_landmarks[idx]
    return lm.x, lm.y


def _xyz(frame_landmarks, idx):
    lm = frame_landmarks[idx]
    return lm.x, lm.y, lm.z


def _dist2d(a, b):
    return math.hypot(a[0] - b[0], a[1] - b[1])


def _angle_deg(a, vertex, b):
    """Angle at vertex in degrees, given three 2D points."""
    va = (a[0] - vertex[0], a[1] - vertex[1])
    vb = (b[0] - vertex[0], b[1] - vertex[1])
    dot = va[0]*vb[0] + va[1]*vb[1]
    mag = math.hypot(*va) * math.hypot(*vb)
    if mag < 1e-9:
        return 0.0
    return math.degrees(math.acos(max(-1.0, min(1.0, dot / mag))))


def _body_height(frame_landmarks):
    """Approximate body height as nose-to-midankle distance (normalized)."""
    nose  = _xy(frame_landmarks, NOSE)
    la    = _xy(frame_landmarks, LEFT_ANKLE)
    ra    = _xy(frame_landmarks, RIGHT_ANKLE)
    mid_ankle = ((la[0]+ra[0])/2, (la[1]+ra[1])/2)
    h = _dist2d(nose, mid_ankle)
    return h if h > 0.05 else 0.5   # fallback if landmarks missing


def compute_metrics(all_landmarks: list, release_frame: int) -> dict:
    """
    Compute all 14 biomechanical metrics.

    Parameters
    ----------
    all_landmarks  : list of mediapipe NormalizedLandmarkList (one per frame)
    release_frame  : index of the release frame

    Returns
    -------
    dict matching the 'features' object in the API response
    """
    n = len(all_landmarks)
    rf = max(0, min(release_frame, n - 1))
    lm_r = all_landmarks[rf]   # landmarks at release frame

    body_h = _body_height(lm_r)

    # ── Arm mechanics ──────────────────────────────────────────────────────────
    r_shoulder = _xy(lm_r, RIGHT_SHOULDER)
    r_elbow    = _xy(lm_r, RIGHT_ELBOW)
    r_wrist    = _xy(lm_r, RIGHT_WRIST)

    # elbow_angle: shoulder→elbow→wrist angle (degrees)
    elbow_angle = _angle_deg(r_shoulder, r_elbow, r_wrist)

    # arm_slot_height: normalized vertical wrist position at release (inverted: 1=top)
    arm_slot_height = max(0.0, 1.0 - r_wrist[1])

    # arm_extension_distance: dist(shoulder, wrist) / body_height
    arm_extension_distance = _dist2d(r_shoulder, r_wrist) / body_h

    # release_height: wrist y normalized (0=bottom, 1=top of frame)
    release_height = max(0.0, 1.0 - r_wrist[1])

    # release_extension: horizontal distance shoulder→wrist / body_height
    release_extension = abs(r_shoulder[0] - r_wrist[0]) / body_h

    # ── Rotational mechanics ───────────────────────────────────────────────────
    l_shoulder = _xy(lm_r, LEFT_SHOULDER)
    l_hip      = _xy(lm_r, LEFT_HIP)
    r_hip      = _xy(lm_r, RIGHT_HIP)

    # shoulder_rotation_angle: angle of shoulder line from horizontal (degrees)
    shoulder_dx = r_shoulder[0] - l_shoulder[0]
    shoulder_dy = r_shoulder[1] - l_shoulder[1]
    shoulder_rotation_angle = abs(math.degrees(math.atan2(shoulder_dy, max(shoulder_dx, 1e-9))))

    # hip_rotation_angle: angle of hip line from horizontal (degrees)
    hip_dx = r_hip[0] - l_hip[0]
    hip_dy = r_hip[1] - l_hip[1]
    hip_rotation_angle = abs(math.degrees(math.atan2(hip_dy, max(hip_dx, 1e-9))))

    # hip_shoulder_separation: difference in rotation angles
    hip_shoulder_separation = abs(shoulder_rotation_angle - hip_rotation_angle)

    # ── Stride mechanics ───────────────────────────────────────────────────────
    # Use foot_plant frame (or release frame fallback) for stride measurement
    fp_idx = max(0, rf - max(1, n // 6))
    lm_fp  = all_landmarks[fp_idx]

    l_ankle_fp = _xy(lm_fp, LEFT_ANKLE)
    r_ankle_fp = _xy(lm_fp, RIGHT_ANKLE)
    body_h_fp  = _body_height(lm_fp)

    # stride_length: ankle-to-ankle distance / body_height at foot plant
    stride_length = _dist2d(l_ankle_fp, r_ankle_fp) / body_h_fp

    # stride_direction_angle: angle of stride line from straight ahead (degrees)
    stride_dx = l_ankle_fp[0] - r_ankle_fp[0]
    stride_dy = l_ankle_fp[1] - r_ankle_fp[1]
    stride_direction_angle = abs(math.degrees(math.atan2(stride_dy, max(abs(stride_dx), 1e-9))))

    # lead_knee_flexion: hip→knee→ankle angle at foot plant
    l_hip_fp   = _xy(lm_fp, LEFT_HIP)
    l_knee_fp  = _xy(lm_fp, LEFT_KNEE)
    lead_knee_flexion = _angle_deg(l_hip_fp, l_knee_fp, l_ankle_fp)
    # Convert to flexion angle (180 = straight, we want bend from straight)
    lead_knee_flexion = max(0.0, 180.0 - lead_knee_flexion)

    # release_lateral_position: wrist x normalized to body width
    r_shoulder_r = _xy(lm_r, RIGHT_SHOULDER)
    l_shoulder_r = _xy(lm_r, LEFT_SHOULDER)
    shoulder_width = _dist2d(l_shoulder_r, r_shoulder_r)
    lateral_center = (l_shoulder_r[0] + r_shoulder_r[0]) / 2.0
    release_lateral_position = (r_wrist[0] - lateral_center) / max(shoulder_width, 0.01)

    # ── Temporal ───────────────────────────────────────────────────────────────
    delivery_tempo = rf / max(n - 1, 1)

    return {
        "elbow_angle":              round(elbow_angle, 2),
        "arm_slot_height":          round(arm_slot_height, 4),
        "arm_extension_distance":   round(min(arm_extension_distance, 1.5), 4),
        "release_height":           round(release_height, 4),
        "hip_shoulder_separation":  round(hip_shoulder_separation, 2),
        "shoulder_rotation_angle":  round(shoulder_rotation_angle, 2),
        "hip_rotation_angle":       round(hip_rotation_angle, 2),
        "release_extension":        round(min(release_extension, 1.5), 4),
        "stride_length":            round(min(stride_length, 1.5), 4),
        "stride_direction_angle":   round(stride_direction_angle, 2),
        "knee_flexion":             round(lead_knee_flexion, 2),
        "release_lateral_position": round(release_lateral_position, 4),
        "delivery_tempo":           round(delivery_tempo, 4),
        "release_frame_index":      rf,
    }
