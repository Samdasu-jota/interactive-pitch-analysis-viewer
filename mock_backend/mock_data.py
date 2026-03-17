"""
Mock data for Interactive Pitch Analysis Viewer.

Generates 60 frames of realistic right-handed pitcher pose data
interpolated through the 5 key delivery phases.
"""

import math

# ── Landmark indices (MediaPipe Pose 33-point model) ──────────────────────────
NOSE           = 0
LEFT_EYE_INNER = 1;  LEFT_EYE = 2;  LEFT_EYE_OUTER = 3
RIGHT_EYE_INNER= 4;  RIGHT_EYE= 5;  RIGHT_EYE_OUTER= 6
LEFT_EAR       = 7;  RIGHT_EAR= 8
MOUTH_LEFT     = 9;  MOUTH_RIGHT = 10
LEFT_SHOULDER  = 11; RIGHT_SHOULDER = 12
LEFT_ELBOW     = 13; RIGHT_ELBOW    = 14
LEFT_WRIST     = 15; RIGHT_WRIST    = 16
LEFT_PINKY     = 17; RIGHT_PINKY    = 18
LEFT_INDEX     = 19; RIGHT_INDEX    = 20
LEFT_THUMB     = 21; RIGHT_THUMB    = 22
LEFT_HIP       = 23; RIGHT_HIP      = 24
LEFT_KNEE      = 25; RIGHT_KNEE     = 26
LEFT_ANKLE     = 27; RIGHT_ANKLE    = 28
LEFT_HEEL      = 29; RIGHT_HEEL     = 30
LEFT_FOOT      = 31; RIGHT_FOOT     = 32

TOTAL_FRAMES = 60
FPS          = 30.0
FRAME_W      = 640
FRAME_H      = 480

# Phase frame indices
PHASE_LEG_LIFT      = 8
PHASE_FOOT_PLANT    = 20
PHASE_MAX_ARM_COCK  = 32
PHASE_RELEASE       = 42
PHASE_FOLLOW_THROUGH= 55


def lerp(a, b, t):
    return a + (b - a) * t

def clamp(v, lo, hi):
    return max(lo, min(hi, v))

def _phase_t(frame, start, end):
    """Normalised [0,1] progress within a phase."""
    if end <= start:
        return 1.0
    return clamp((frame - start) / (end - start), 0.0, 1.0)


# ── Base landmark positions (right-handed pitcher, set position) ──────────────
# Coordinates are normalised [0,1] relative to 640×480 frame.
# Origin: top-left.  x: left→right,  y: top→bottom.
# Pitcher faces camera (catcher's POV). Right arm = throwing arm.

def _base_landmarks():
    """34-element list of [x, y, z, vis] for the set/windup position."""
    lm = [[0.5, 0.5, 0.0, 0.9]] * 33   # defaults

    # Head / face cluster (near top-center of frame)
    lm[NOSE]           = [0.50, 0.13, 0.00, 0.99]
    lm[LEFT_EYE_INNER] = [0.48, 0.12, 0.00, 0.95]
    lm[LEFT_EYE]       = [0.47, 0.11, 0.00, 0.95]
    lm[LEFT_EYE_OUTER] = [0.46, 0.11, 0.00, 0.90]
    lm[RIGHT_EYE_INNER]= [0.52, 0.12, 0.00, 0.95]
    lm[RIGHT_EYE]      = [0.53, 0.11, 0.00, 0.95]
    lm[RIGHT_EYE_OUTER]= [0.54, 0.11, 0.00, 0.90]
    lm[LEFT_EAR]       = [0.44, 0.14, 0.00, 0.85]
    lm[RIGHT_EAR]      = [0.56, 0.14, 0.00, 0.85]
    lm[MOUTH_LEFT]     = [0.48, 0.16, 0.00, 0.85]
    lm[MOUTH_RIGHT]    = [0.52, 0.16, 0.00, 0.85]

    # Shoulders (mid-frame, slightly wide)
    lm[LEFT_SHOULDER]  = [0.42, 0.28, 0.00, 0.98]
    lm[RIGHT_SHOULDER] = [0.58, 0.28, 0.00, 0.98]

    # Left arm — glove side, slightly raised
    lm[LEFT_ELBOW]     = [0.34, 0.36, 0.02, 0.96]
    lm[LEFT_WRIST]     = [0.30, 0.44, 0.04, 0.94]
    lm[LEFT_PINKY]     = [0.28, 0.46, 0.04, 0.80]
    lm[LEFT_INDEX]     = [0.29, 0.47, 0.04, 0.80]
    lm[LEFT_THUMB]     = [0.31, 0.46, 0.03, 0.80]

    # Right arm — throwing side, tucked into glove
    lm[RIGHT_ELBOW]    = [0.62, 0.36, 0.02, 0.96]
    lm[RIGHT_WRIST]    = [0.54, 0.44, 0.04, 0.94]
    lm[RIGHT_PINKY]    = [0.53, 0.46, 0.04, 0.80]
    lm[RIGHT_INDEX]    = [0.54, 0.47, 0.04, 0.80]
    lm[RIGHT_THUMB]    = [0.55, 0.46, 0.03, 0.80]

    # Hips
    lm[LEFT_HIP]       = [0.45, 0.52, 0.00, 0.98]
    lm[RIGHT_HIP]      = [0.55, 0.52, 0.00, 0.98]

    # Left leg — stride leg
    lm[LEFT_KNEE]      = [0.44, 0.66, 0.00, 0.96]
    lm[LEFT_ANKLE]     = [0.43, 0.80, 0.00, 0.94]
    lm[LEFT_HEEL]      = [0.42, 0.82, 0.00, 0.88]
    lm[LEFT_FOOT]      = [0.44, 0.85, 0.00, 0.88]

    # Right leg — push leg
    lm[RIGHT_KNEE]     = [0.56, 0.66, 0.00, 0.96]
    lm[RIGHT_ANKLE]    = [0.57, 0.80, 0.00, 0.94]
    lm[RIGHT_HEEL]     = [0.58, 0.82, 0.00, 0.88]
    lm[RIGHT_FOOT]     = [0.56, 0.85, 0.00, 0.88]

    return lm


def _generate_frame_landmarks(frame):
    """
    Returns 33 landmarks for the given frame, animating through delivery phases.
    Phases:
      0–7:   Windup
      8–19:  Leg lift (left knee rises)
      20–31: Stride (left foot plants forward)
      32–41: Arm cock (throwing arm goes back & up)
      42:    Release
      43–59: Follow-through
    """
    base = _base_landmarks()
    lm   = [list(p) for p in base]   # deep copy

    # ── Windup (0→7) ──────────────────────────────────────────────────────────
    if frame <= PHASE_LEG_LIFT:
        t = _phase_t(frame, 0, PHASE_LEG_LIFT)
        # Slight weight shift to right, arms rise together
        shift = 0.03 * math.sin(t * math.pi)
        for idx in [LEFT_SHOULDER, RIGHT_SHOULDER,
                    LEFT_ELBOW, RIGHT_ELBOW,
                    LEFT_WRIST, RIGHT_WRIST]:
            lm[idx][0] += shift
            lm[idx][1] -= 0.02 * t   # arms rise slightly

    # ── Leg Lift (8→19) ───────────────────────────────────────────────────────
    elif frame <= PHASE_FOOT_PLANT:
        t = _phase_t(frame, PHASE_LEG_LIFT, PHASE_FOOT_PLANT)
        lift = math.sin(t * math.pi)   # rises then comes down at end of lift

        # Left knee rises toward chest
        lm[LEFT_KNEE][1]  = base[LEFT_KNEE][1]  - 0.22 * lift
        lm[LEFT_ANKLE][1] = base[LEFT_ANKLE][1] - 0.28 * lift
        lm[LEFT_HEEL][1]  = base[LEFT_HEEL][1]  - 0.28 * lift
        lm[LEFT_FOOT][1]  = base[LEFT_FOOT][1]  - 0.25 * lift
        lm[LEFT_KNEE][0]  = base[LEFT_KNEE][0]  + 0.04 * lift
        lm[LEFT_ANKLE][0] = base[LEFT_ANKLE][0] + 0.04 * lift

        # Torso tilts slightly toward first base (left for RHP from front)
        tilt = 0.03 * t
        for idx in [LEFT_SHOULDER, LEFT_HIP, LEFT_ELBOW]:
            lm[idx][1] -= tilt
        for idx in [RIGHT_SHOULDER, RIGHT_HIP, RIGHT_ELBOW]:
            lm[idx][1] += tilt * 0.5

    # ── Stride (20→31) ────────────────────────────────────────────────────────
    elif frame <= PHASE_MAX_ARM_COCK:
        t = _phase_t(frame, PHASE_FOOT_PLANT, PHASE_MAX_ARM_COCK)

        # Left foot strides forward (toward camera = lower y in 2D projection)
        stride = t
        lm[LEFT_KNEE][1]  = base[LEFT_KNEE][1]  - 0.04 * stride
        lm[LEFT_ANKLE][1] = base[LEFT_ANKLE][1] + 0.02 * stride
        lm[LEFT_HEEL][1]  = base[LEFT_HEEL][1]  + 0.02 * stride
        lm[LEFT_FOOT][1]  = base[LEFT_FOOT][1]  + 0.02 * stride
        lm[LEFT_ANKLE][0] = base[LEFT_ANKLE][0] - 0.10 * stride   # strides left
        lm[LEFT_KNEE][0]  = base[LEFT_KNEE][0]  - 0.08 * stride
        lm[LEFT_FOOT][0]  = base[LEFT_FOOT][0]  - 0.10 * stride

        # Hip opens: left hip rotates toward camera
        lm[LEFT_HIP][0]   = base[LEFT_HIP][0]   - 0.04 * stride
        lm[RIGHT_HIP][0]  = base[RIGHT_HIP][0]  + 0.02 * stride

        # Throwing arm begins to separate
        lm[RIGHT_ELBOW][0] = base[RIGHT_ELBOW][0] + 0.05 * stride
        lm[RIGHT_WRIST][0] = base[RIGHT_WRIST][0] + 0.08 * stride
        lm[RIGHT_ELBOW][1] = base[RIGHT_ELBOW][1] - 0.04 * stride
        lm[RIGHT_WRIST][1] = base[RIGHT_WRIST][1] - 0.02 * stride

        # Glove arm extends toward target
        lm[LEFT_ELBOW][0] = base[LEFT_ELBOW][0] - 0.05 * stride
        lm[LEFT_WRIST][0] = base[LEFT_WRIST][0] - 0.08 * stride

    # ── Arm Cock (32→41) ──────────────────────────────────────────────────────
    elif frame <= PHASE_RELEASE:
        t = _phase_t(frame, PHASE_MAX_ARM_COCK, PHASE_RELEASE)

        # Fully strided left leg — planted
        lm[LEFT_ANKLE][0] = base[LEFT_ANKLE][0] - 0.10
        lm[LEFT_KNEE][0]  = base[LEFT_KNEE][0]  - 0.08
        lm[LEFT_KNEE][1]  = base[LEFT_KNEE][1]  - 0.04

        # Throwing arm cocks: elbow up & back, wrist high
        cock_back = math.sin((1 - t) * math.pi * 0.5)
        lm[RIGHT_ELBOW][0] = base[RIGHT_ELBOW][0] + 0.10 - 0.10 * t
        lm[RIGHT_ELBOW][1] = base[RIGHT_ELBOW][1] - 0.12 + 0.06 * t
        lm[RIGHT_WRIST][0] = base[RIGHT_WRIST][0] + 0.12 - 0.16 * t
        lm[RIGHT_WRIST][1] = base[RIGHT_WRIST][1] - 0.16 + 0.20 * t

        # Hip/shoulder separation: hips are open, shoulders lag
        lm[LEFT_HIP][0]       = base[LEFT_HIP][0]   - 0.04
        lm[RIGHT_HIP][0]      = base[RIGHT_HIP][0]  + 0.04
        lm[LEFT_SHOULDER][0]  = base[LEFT_SHOULDER][0]  - 0.02 * t
        lm[RIGHT_SHOULDER][0] = base[RIGHT_SHOULDER][0] + 0.02 * t

        # Glove arm tucking
        lm[LEFT_ELBOW][0] = base[LEFT_ELBOW][0] - 0.05 + 0.04 * t
        lm[LEFT_WRIST][0] = base[LEFT_WRIST][0] - 0.06 + 0.05 * t
        lm[LEFT_ELBOW][1] = base[LEFT_ELBOW][1] + 0.04 * t
        lm[LEFT_WRIST][1] = base[LEFT_WRIST][1] + 0.06 * t

        # Lead knee flexion at foot plant
        lm[LEFT_KNEE][1] = base[LEFT_KNEE][1] + 0.04 * t

    # ── Follow-Through (43→59) ────────────────────────────────────────────────
    else:
        t = _phase_t(frame, PHASE_RELEASE, PHASE_FOLLOW_THROUGH)

        # Release position: arm fully extended toward target
        lm[RIGHT_ELBOW][0] = base[RIGHT_ELBOW][0] - 0.08 * t
        lm[RIGHT_ELBOW][1] = base[RIGHT_ELBOW][1] + 0.10 * t
        lm[RIGHT_WRIST][0] = base[RIGHT_WRIST][0] - 0.14 * t
        lm[RIGHT_WRIST][1] = base[RIGHT_WRIST][1] + 0.18 * t

        # Body finishes rotating; trunk flexes forward
        for idx in [NOSE, LEFT_SHOULDER, RIGHT_SHOULDER]:
            lm[idx][1] += 0.06 * t

        # Left leg posts; right leg swings through
        lm[LEFT_KNEE][0]  = base[LEFT_KNEE][0] - 0.08
        lm[LEFT_ANKLE][0] = base[LEFT_ANKLE][0] - 0.10
        lm[RIGHT_KNEE][0] = base[RIGHT_KNEE][0] - 0.04 * t
        lm[RIGHT_ANKLE][0]= base[RIGHT_ANKLE][0]- 0.06 * t

    # Clamp all coordinates
    for i in range(33):
        lm[i][0] = clamp(lm[i][0], 0.01, 0.99)
        lm[i][1] = clamp(lm[i][1], 0.01, 0.99)

    return [
        {"x": float(p[0]), "y": float(p[1]), "z": float(p[2]), "visibility": float(p[3])}
        for p in lm
    ]


# ── Build full pose sequence ───────────────────────────────────────────────────

def _build_poses():
    poses = []
    for f in range(TOTAL_FRAMES):
        poses.append({
            "frame_number": f,
            "frame_shape": [FRAME_H, FRAME_W],
            "landmarks": _generate_frame_landmarks(f)
        })
    return poses


# ── Analysis response ──────────────────────────────────────────────────────────

ANALYSIS_RESPONSE = {
    "total_frames": TOTAL_FRAMES,
    "detected_frames": TOTAL_FRAMES,
    "poses": _build_poses(),
    "features": {
        "elbow_angle":              87.3,   # optimal: 85–100°
        "arm_slot_height":          0.48,
        "arm_extension_distance":   0.91,   # optimal: 0.85–1.0
        "release_height":           0.47,   # optimal: 0.30–0.55
        "hip_shoulder_separation":  54.2,   # optimal: 45–65°
        "shoulder_rotation_angle":  98.7,   # optimal: 80–110°
        "hip_rotation_angle":       44.5,
        "release_extension":        0.88,
        "stride_length":            0.83,   # optimal: 0.75–0.95
        "stride_direction_angle":    5.2,
        "knee_flexion":             42.1,   # optimal: 30–55°
        "release_lateral_position":  0.12,
        "delivery_tempo":           0.64,   # optimal: 0.55–0.70
        "release_frame_index":      PHASE_RELEASE,
    },
    "phases": {
        "leg_lift":      PHASE_LEG_LIFT,
        "foot_plant":    PHASE_FOOT_PLANT,
        "max_arm_cock":  PHASE_MAX_ARM_COCK,
        "release_point": PHASE_RELEASE,
        "follow_through":PHASE_FOLLOW_THROUGH,
    },
    "recommendations": [
        {
            "pitcher_name": "Gerrit Cole",
            "throws": "right",
            "similarity_score": "87.5%",
            "summary": "Strong mechanical similarity. Focus on earlier hip engagement.",
            "similar_mechanics": [
                "Elbow angle at release (87° vs 92°)",
                "Release height (normalized 0.47 vs 0.49)",
                "Stride length (0.83 vs 0.86 × body height)",
            ],
            "coaching_cues": [
                "Initiate hip rotation 8–10% earlier relative to foot plant",
                "Maintain 90° shoulder abduction through acceleration phase",
                "Drive back knee toward plate to maximize hip extension",
            ],
            "notable_differences": [
                "Slightly open elbow — Cole averages 92° at release",
                "Hip-shoulder separation 3° lower than Cole's 57° average",
            ],
        },
        {
            "pitcher_name": "Clayton Kershaw",
            "throws": "left",
            "similarity_score": "79.2%",
            "summary": "Efficient arm path and stride mechanics. Work on shoulder rotation timing.",
            "similar_mechanics": [
                "Delivery tempo (0.64 vs 0.65 fraction)",
                "Arm extension (0.91 vs 0.93 normalized)",
                "Knee flexion at foot plant (42° vs 44°)",
            ],
            "coaching_cues": [
                "Delay shoulder rotation 5% longer to increase hip-shoulder separation",
                "Steepen arm slot slightly for more downward plane",
            ],
            "notable_differences": [
                "Shoulder rotation 6° less than Kershaw's 105° peak",
                "Stride direction 2° more closed than Kershaw's line",
            ],
        },
        {
            "pitcher_name": "Yu Darvish",
            "throws": "right",
            "similarity_score": "71.8%",
            "summary": "Good rotational power. Arm extension and follow-through can be improved.",
            "similar_mechanics": [
                "Hip rotation angle at release (44° vs 46°)",
                "Stride length (0.83 vs 0.81 × body height)",
            ],
            "coaching_cues": [
                "Extend arm 5% further through the release zone",
                "Allow trunk to flex more aggressively post-release",
                "Land stride foot 3–4 cm more toward third-base side",
            ],
            "notable_differences": [
                "Arm extension 4% less than Darvish's 0.95 average",
                "Follow-through deceleration starts 3 frames earlier",
            ],
        },
    ],
}


# ── Pitcher database ───────────────────────────────────────────────────────────

_VIDEO_DIR = "/Users/javis/Desktop/Projects/Interactive Pitch Analysis Viewer/videos"

PITCHER_LIST = [
    {
        "id": "gerrit_cole",
        "name": "Gerrit Cole",
        "team": "New York Yankees",
        "throws": "right",
        "video_url": f"file://{_VIDEO_DIR}/kershaw.mp4",  # fallback
    },
    {
        "id": "clayton_kershaw",
        "name": "Clayton Kershaw",
        "team": "Los Angeles Dodgers",
        "throws": "left",
        "video_url": f"file://{_VIDEO_DIR}/kershaw.mp4",
    },
    {
        "id": "yu_darvish",
        "name": "Yu Darvish",
        "team": "San Diego Padres",
        "throws": "right",
        "video_url": f"file://{_VIDEO_DIR}/darvish.mp4",
    },
    {
        "id": "shohei_ohtani",
        "name": "Shohei Ohtani",
        "team": "Los Angeles Dodgers",
        "throws": "right",
        "video_url": f"file://{_VIDEO_DIR}/darvish.mp4",  # fallback
    },
    {
        "id": "sandy_alcantara",
        "name": "Sandy Alcantara",
        "team": "Miami Marlins",
        "throws": "right",
        "video_url": f"file://{_VIDEO_DIR}/darvish.mp4",  # fallback
    },
]

PITCHER_MAP = {p["id"]: p for p in PITCHER_LIST}
