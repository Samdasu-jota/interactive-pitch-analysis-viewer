"""
Heuristic pitch phase detection from MediaPipe landmark sequences.

All landmark coordinates are normalized [0,1] (MediaPipe convention).
In image space: y increases downward, so a higher knee = smaller y value.
"""

import numpy as np
from typing import List, Dict

# MediaPipe Pose landmark indices
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
LEFT_EAR       = 7
RIGHT_EAR      = 8


def _lm(frame_landmarks, idx):
    """Return (x, y, visibility) for landmark idx in a frame."""
    lm = frame_landmarks[idx]
    return lm.x, lm.y, lm.visibility


def detect_phases(all_landmarks: list, fps: float = 30.0) -> Dict[str, int]:
    """
    Detect key pitch phase frame indices from a sequence of MediaPipe landmarks.

    Parameters
    ----------
    all_landmarks : list of mediapipe NormalizedLandmarkList
    fps           : video frame rate

    Returns
    -------
    dict with keys: leg_lift, foot_plant, max_arm_cock, release_point, follow_through
    All values are frame indices (-1 if not detected).
    """
    n = len(all_landmarks)
    if n < 5:
        return dict(leg_lift=-1, foot_plant=-1, max_arm_cock=-1,
                    release_point=-1, follow_through=-1)

    # Extract per-frame values as arrays
    left_knee_y   = np.array([_lm(f, LEFT_KNEE)[1]   for f in all_landmarks])
    left_ankle_y  = np.array([_lm(f, LEFT_ANKLE)[1]  for f in all_landmarks])
    right_wrist_x = np.array([_lm(f, RIGHT_WRIST)[0] for f in all_landmarks])
    right_wrist_y = np.array([_lm(f, RIGHT_WRIST)[1] for f in all_landmarks])
    right_ear_y   = np.array([_lm(f, RIGHT_EAR)[1]   for f in all_landmarks])
    right_shoulder_x = np.array([_lm(f, RIGHT_SHOULDER)[0] for f in all_landmarks])

    # ── 1. Leg lift: frame where left knee is highest (min y in image coords) ──
    # Only search first 60% of video
    search_end = max(1, int(n * 0.6))
    leg_lift = int(np.argmin(left_knee_y[:search_end]))

    # ── 2. Foot plant: after leg_lift, ankle y stabilises (derivative ≈ 0) ────
    foot_plant = -1
    if leg_lift < n - 5:
        ankle_vel = np.abs(np.gradient(left_ankle_y[leg_lift:]))
        # Find first frame after peak where ankle slows significantly
        stable_frames = np.where(ankle_vel < 0.005)[0]
        if len(stable_frames) > 0:
            foot_plant = leg_lift + int(stable_frames[0])
        else:
            # Fallback: 40% after leg_lift
            foot_plant = leg_lift + max(1, int((n - leg_lift) * 0.4))

    start = foot_plant if foot_plant > 0 else leg_lift

    # ── 3. Max arm cock: right wrist at or above ear level, behind shoulder ───
    max_arm_cock = -1
    best_cock_score = -1.0
    search_start = max(0, start)
    search_end2  = min(n, search_start + int(n * 0.5))
    for i in range(search_start, search_end2):
        wrist_above_ear = right_ear_y[i] - right_wrist_y[i]   # positive = wrist above ear
        wrist_behind_shoulder = right_wrist_x[i] - right_shoulder_x[i]  # positive = behind
        score = wrist_above_ear + wrist_behind_shoulder * 0.5
        if score > best_cock_score:
            best_cock_score = score
            max_arm_cock = i

    if max_arm_cock < 0:
        max_arm_cock = start + max(1, int((n - start) * 0.3))

    # ── 4. Release point: max forward velocity of right wrist ─────────────────
    release_point = -1
    if max_arm_cock < n - 3:
        wrist_vel_x = np.gradient(right_wrist_x)  # positive = moving right (toward plate)
        # Invert: in front-facing camera, pitcher's wrist moves in -x direction at release
        wrist_vel_neg = -wrist_vel_x
        search_start3 = max_arm_cock
        search_end3   = min(n - 1, max_arm_cock + int(n * 0.4))
        if search_end3 > search_start3:
            release_point = search_start3 + int(
                np.argmax(wrist_vel_neg[search_start3:search_end3])
            )

    if release_point < 0 or release_point <= max_arm_cock:
        release_point = max_arm_cock + max(1, int((n - max_arm_cock) * 0.4))

    # ── 5. Follow-through: ~12 frames after release ───────────────────────────
    follow_through = min(n - 1, release_point + 12)

    return dict(
        leg_lift=leg_lift,
        foot_plant=foot_plant,
        max_arm_cock=max_arm_cock,
        release_point=release_point,
        follow_through=follow_through,
    )
