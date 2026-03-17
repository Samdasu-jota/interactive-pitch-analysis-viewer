from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.units import cm
from reportlab.lib import colors
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Table, TableStyle,
    HRFlowable, KeepTogether
)
from reportlab.lib.enums import TA_LEFT, TA_CENTER, TA_JUSTIFY

OUTPUT = "/Users/javis/Desktop/Projects/Interactive Pitch Analysis Viewer/Interactive_Pitch_Analysis_Viewer.pdf"

# ── Colours ────────────────────────────────────────────────────────────────────
C_DARK   = colors.HexColor("#1A1A2E")
C_BLUE   = colors.HexColor("#0F3460")
C_ACCENT = colors.HexColor("#E94560")
C_TEAL   = colors.HexColor("#16213E")
C_LIGHT  = colors.HexColor("#F0F0F0")
C_GREEN  = colors.HexColor("#4CAF50")
C_AMBER  = colors.HexColor("#FFC107")
C_WHITE  = colors.white
C_GRAY   = colors.HexColor("#888888")
C_CODE   = colors.HexColor("#2D2D2D")

doc = SimpleDocTemplate(
    OUTPUT,
    pagesize=A4,
    rightMargin=2*cm, leftMargin=2*cm,
    topMargin=2*cm, bottomMargin=2*cm,
    title="Interactive Pitch Analysis Viewer — Technical Overview",
    author="Portfolio Project",
)

W = A4[0] - 4*cm   # usable width

# ── Styles ─────────────────────────────────────────────────────────────────────
base = getSampleStyleSheet()

def style(name, parent="Normal", **kw):
    s = ParagraphStyle(name, parent=base[parent], **kw)
    return s

S = {
    "cover_title": style("cover_title",
        fontSize=28, textColor=C_WHITE, leading=36,
        spaceAfter=8, alignment=TA_CENTER, fontName="Helvetica-Bold"),
    "cover_sub":   style("cover_sub",
        fontSize=14, textColor=C_ACCENT, leading=20,
        spaceAfter=4, alignment=TA_CENTER),
    "cover_meta":  style("cover_meta",
        fontSize=10, textColor=C_LIGHT, leading=14,
        spaceAfter=2, alignment=TA_CENTER),
    "h1": style("h1", fontSize=18, textColor=C_ACCENT, leading=24,
        spaceBefore=18, spaceAfter=6, fontName="Helvetica-Bold"),
    "h2": style("h2", fontSize=13, textColor=C_WHITE, leading=18,
        spaceBefore=14, spaceAfter=4, fontName="Helvetica-Bold"),
    "h3": style("h3", fontSize=11, textColor=C_AMBER, leading=15,
        spaceBefore=10, spaceAfter=3, fontName="Helvetica-Bold"),
    "body": style("body", fontSize=9.5, textColor=C_LIGHT, leading=14,
        spaceAfter=5, alignment=TA_JUSTIFY),
    "bullet": style("bullet", fontSize=9.5, textColor=C_LIGHT, leading=14,
        spaceAfter=3, leftIndent=14, bulletIndent=4),
    "code_inline": style("code_inline", fontSize=8.5, textColor=C_AMBER,
        fontName="Courier", leading=12),
    "label": style("label", fontSize=8, textColor=C_GRAY, leading=11,
        fontName="Helvetica-Bold"),
    "caption": style("caption", fontSize=8.5, textColor=C_GRAY, leading=12,
        alignment=TA_CENTER, spaceAfter=4),
}

def hr(color=C_BLUE, thickness=1):
    return HRFlowable(width="100%", thickness=thickness, color=color, spaceAfter=6, spaceBefore=2)

def h1(text): return Paragraph(text, S["h1"])
def h2(text): return Paragraph(text, S["h2"])
def h3(text): return Paragraph(text, S["h3"])
def body(text): return Paragraph(text, S["body"])
def bullet(text): return Paragraph(f"• {text}", S["bullet"])
def sp(n=8): return Spacer(1, n)

def code(text):
    return Paragraph(f'<font name="Courier" color="#FFC107">{text}</font>', S["code_inline"])

def section_table(rows, col_widths=None):
    if col_widths is None:
        col_widths = [W * 0.28, W * 0.72]
    t = Table(rows, colWidths=col_widths, hAlign="LEFT")
    t.setStyle(TableStyle([
        ("BACKGROUND",   (0, 0), (0, -1), C_TEAL),
        ("BACKGROUND",   (1, 0), (1, -1), C_CODE),
        ("TEXTCOLOR",    (0, 0), (0, -1), C_AMBER),
        ("TEXTCOLOR",    (1, 0), (1, -1), C_LIGHT),
        ("FONTNAME",     (0, 0), (-1, -1), "Helvetica"),
        ("FONTNAME",     (0, 0), (0, -1), "Helvetica-Bold"),
        ("FONTSIZE",     (0, 0), (-1, -1), 8.5),
        ("LEADING",      (0, 0), (-1, -1), 13),
        ("TOPPADDING",   (0, 0), (-1, -1), 5),
        ("BOTTOMPADDING",(0, 0), (-1, -1), 5),
        ("LEFTPADDING",  (0, 0), (-1, -1), 8),
        ("RIGHTPADDING", (0, 0), (-1, -1), 8),
        ("ROWBACKGROUNDS",(0,0),(-1,-1),[C_TEAL, colors.HexColor("#0D1B2A")]),
        ("GRID",         (0, 0), (-1, -1), 0.4, colors.HexColor("#333366")),
        ("VALIGN",       (0, 0), (-1, -1), "TOP"),
    ]))
    return t

def header_table(rows, col_widths=None):
    """Table with a dark header row."""
    if col_widths is None:
        col_widths = [W * 0.25, W * 0.45, W * 0.30]
    t = Table(rows, colWidths=col_widths, hAlign="LEFT")
    t.setStyle(TableStyle([
        ("BACKGROUND",   (0, 0), (-1, 0), C_BLUE),
        ("BACKGROUND",   (0, 1), (-1, -1), C_CODE),
        ("ROWBACKGROUNDS",(0,1),(-1,-1),[C_CODE, C_TEAL]),
        ("TEXTCOLOR",    (0, 0), (-1, 0), C_WHITE),
        ("TEXTCOLOR",    (0, 1), (-1, -1), C_LIGHT),
        ("FONTNAME",     (0, 0), (-1, 0), "Helvetica-Bold"),
        ("FONTNAME",     (0, 1), (-1, -1), "Helvetica"),
        ("FONTSIZE",     (0, 0), (-1, -1), 8.5),
        ("LEADING",      (0, 0), (-1, -1), 13),
        ("TOPPADDING",   (0, 0), (-1, -1), 5),
        ("BOTTOMPADDING",(0, 0), (-1, -1), 5),
        ("LEFTPADDING",  (0, 0), (-1, -1), 8),
        ("RIGHTPADDING", (0, 0), (-1, -1), 8),
        ("GRID",         (0, 0), (-1, -1), 0.4, colors.HexColor("#333366")),
        ("VALIGN",       (0, 0), (-1, -1), "TOP"),
        ("ALIGN",        (0, 0), (-1, 0), "CENTER"),
    ]))
    return t

# ── Cover page ─────────────────────────────────────────────────────────────────
cover_bg = Table(
    [[Paragraph("Interactive Pitch Analysis Viewer", S["cover_title"])],
     [Paragraph("C++ Qt6 · OpenCV · Python FastAPI · MediaPipe", S["cover_sub"])],
     [Spacer(1, 10)],
     [Paragraph("Technical Design & Architecture Overview", S["cover_meta"])],
     [Paragraph("Portfolio Project — Tesla Vehicle UI Internship", S["cover_meta"])],
     [Spacer(1, 6)],
     [Paragraph("Version 1.0.0  ·  2026", S["cover_meta"])],
    ],
    colWidths=[W],
)
cover_bg.setStyle(TableStyle([
    ("BACKGROUND",    (0, 0), (-1, -1), C_DARK),
    ("TOPPADDING",    (0, 0), (-1, -1), 14),
    ("BOTTOMPADDING", (0, 0), (-1, -1), 14),
    ("LEFTPADDING",   (0, 0), (-1, -1), 20),
    ("RIGHTPADDING",  (0, 0), (-1, -1), 20),
    ("ALIGN",         (0, 0), (-1, -1), "CENTER"),
    ("VALIGN",        (0, 0), (-1, -1), "MIDDLE"),
]))

# ── Document flow ──────────────────────────────────────────────────────────────
story = []

# Cover
story.append(cover_bg)
story.append(sp(20))
story.append(hr(C_ACCENT, 2))

# ── 1. Project Overview ────────────────────────────────────────────────────────
story.append(h1("1. Project Overview"))
story.append(hr())
story.append(body(
    "The <b>Interactive Pitch Analysis Viewer</b> is a desktop application that "
    "enables coaches and players to upload a baseball pitching video, receive AI-driven "
    "biomechanical analysis from a Python backend, and interactively explore the results "
    "frame-by-frame with a skeleton overlay and 14 quantified metrics. A side-by-side "
    "comparison feature lets users visually diff their mechanics against professional pitchers "
    "retrieved from a backend database."
))
story.append(sp(6))

story.append(section_table([
    ["Language",      "C++17"],
    ["UI Framework",  "Qt 6 (Core, Widgets, Network, Concurrent)"],
    ["Video Decode",  "OpenCV 4  —  cv::VideoCapture"],
    ["Pose Data",     "MediaPipe Pose  (33 keypoints, via Python backend)"],
    ["AI Backend",    "Python FastAPI @ localhost:8000"],
    ["JSON",          "nlohmann/json (header-only, bundled in third_party/)"],
    ["Build System",  "CMake 3.22+ with Ninja, CMakePresets.json (debug / release)"],
    ["Architecture",  "Model-View-Presenter (MVP)  +  Service layer"],
    ["Threading",     "UI thread + dedicated VideoDecodeService QThread"],
], col_widths=[W*0.30, W*0.70]))

# ── 2. High-Level Architecture ─────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("2. High-Level Architecture"))
story.append(hr())
story.append(body(
    "The application follows a strict <b>Model-View-Presenter (MVP)</b> pattern. "
    "The Model is the single source of truth (SessionModel). Views are passive — they only "
    "render state and forward user input. Presenters contain all business logic and mediate "
    "between Views and Models. Services run independently (often on separate threads) and "
    "communicate results back to the UI thread exclusively through Qt signals/slots with "
    "<b>Qt::QueuedConnection</b>, guaranteeing thread safety."
))
story.append(sp(6))

arch_rows = [
    ["Layer",        "Classes",                         "Responsibility"],
    ["App Bootstrap","Application, MainWindow,\nAppConfig",
                                                        "Entry point, window shell, persisted config via QSettings"],
    ["Presenters",   "MainPresenter, VideoPresenter,\nAnalysisPresenter, ComparisonPresenter",
                                                        "Business logic, workflow coordination, bind Views ↔ Model"],
    ["Models",       "SessionModel, AnalysisResult,\nBiomechanicsMetrics, PoseFrame,\nFrameCache",
                                                        "Application state, data structures, LRU frame cache"],
    ["Views",        "VideoPlayerWidget, TimelineWidget,\nMetricsPanel, MetricCard,\nComparisonView, PitcherMatchPanel",
                                                        "Passive UI widgets — render state, emit user input signals"],
    ["Rendering",    "PoseOverlayRenderer,\nIMetricRenderer (+3 impls)",
                                                        "Stateless drawing helpers; Strategy pattern for metric gauges"],
    ["Services",     "VideoDecodeService, ApiClient,\nAnalysisThread, PrefetchService,\nThumbnailGenerator",
                                                        "Background work: video decode, HTTP, prefetch"],
    ["Utils",        "FrameRequestQueue, JsonParser,\nCoordMapper, SkeletonTopology,\nMetricThresholds",
                                                        "Thread-safe queue, JSON parsing, coordinate mapping, thresholds"],
]
story.append(header_table(arch_rows, col_widths=[W*0.22, W*0.33, W*0.45]))

# ── 3. Data Flow ───────────────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("3. Data Flow — End-to-End"))
story.append(hr())

story.append(h2("3.1  Video Loading"))
for t in [
    "User drags a video file onto MainWindow or selects via File → Open.",
    "MainPresenter.loadVideo() is called → creates VideoDecodeService on a new QThread.",
    "VideoDecodeService opens the file with cv::VideoCapture, emits videoMetadataReady (totalFrames, fps, resolution).",
    "MainPresenter updates SessionModel.setVideoMetadata() → SessionModel emits videoLoaded.",
    "VideoPresenter receives videoLoaded → configures TimelineWidget, requests frame 0.",
    "VideoDecodeService decodes frame 0 → emits frameReady(QImage, 0) via QueuedConnection.",
    "VideoPlayerWidget.onFrameReady() stores the image → calls update() → paintEvent() renders it.",
]:
    story.append(bullet(t))

story.append(sp(6))
story.append(h2("3.2  Analysis Request"))
for t in [
    "User clicks Analyze → MainPresenter.startAnalysis() → delegates to AnalysisPresenter.",
    "AnalysisPresenter spawns AnalysisThread. Thread calls ApiClient.uploadVideoForAnalysis().",
    "ApiClient performs multipart POST to /analyze/upload. Progress signals drive QProgressBar.",
    "On HTTP 200: ApiClient emits analysisResponseReceived(QByteArray json).",
    "JsonParser (static, no state) parses JSON → AnalysisResult struct.",
    "AnalysisPresenter writes result to SessionModel.setAnalysisResult().",
    "SessionModel emits analysisResultReady → MetricsPanel, PitcherMatchPanel, TimelineWidget update.",
    "VideoPresenter observes currentFrameChanged → looks up PoseFrame → pushes to VideoPlayerWidget.",
]:
    story.append(bullet(t))

story.append(sp(6))
story.append(h2("3.3  Playback & Scrubbing"))
for t in [
    "VideoPresenter runs a QTimer at target fps. Each tick: increments frame, calls VideoDecodeService.requestFrame().",
    "requestFrame() pushes the frame number into FrameRequestQueue (thread-safe, condition_variable).",
    "Decode thread wakes, calls popLatest() — drains the entire queue, processes only the LATEST request.",
    "Decoded QImage is stored in FrameCache (LRU, QReadWriteLock, 60-frame capacity).",
    "frameReady signal crosses thread boundary (QueuedConnection) → VideoPlayerWidget re-renders.",
    "During rapid scrubbing, stale intermediate frames are discarded — only the most recent request matters.",
]:
    story.append(bullet(t))

# ── 4. Key Design Decisions ────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("4. Key Design Decisions & Rationale"))
story.append(hr())

decisions = [
    ("Latest-Only Queue\n(FrameRequestQueue)",
     "cv::VideoCapture requires sequential seeking, which is slow. During fast scrubbing, the "
     "decode thread cannot keep up with UI events. Discarding all but the latest request prevents "
     "queue buildup and ensures the user always sees the frame they are currently on — identical "
     "to the technique used in real-time sensor fusion pipelines where stale data is harmful."),
    ("LRU FrameCache\n(FrameCache)",
     "Random-access video seeking is expensive (I-frame → P-frame decode chain). Caching the "
     "most recently accessed 60 frames (~2 seconds at 30fps) eliminates re-decode on back-scrub. "
     "Implemented as std::list (LRU order) + std::unordered_map (O(1) lookup). "
     "QReadWriteLock allows concurrent UI-thread reads while the decode thread writes."),
    ("Stateless PoseOverlayRenderer",
     "Drawing code has zero member state — all parameters are passed as const arguments. "
     "This eliminates any need for thread-safety analysis and makes the renderer trivially unit-testable. "
     "It also allows the same renderer instance to be reused for both the main view and the comparison view."),
    ("Strategy Pattern for Metric Gauges\n(IMetricRenderer)",
     "The 14 biomechanical metrics divide into three visual categories: angles (bar gauge with °), "
     "ratios/normalised values (bar gauge with decimal), and tempo (temporal fraction). "
     "IMetricRenderer is a pure-virtual interface; each MetricCard holds a unique_ptr<IMetricRenderer>. "
     "Adding a new visualization requires only a new subclass — MetricCard is never modified."),
    ("SessionModel as Single Source of Truth",
     "Rather than passing data directly between widgets, all state lives in SessionModel. "
     "Every view subscribes to SessionModel signals. This prevents inconsistent UI state "
     "(e.g., the timeline shows a different frame than the video player) and makes the state "
     "trivially inspectable during debugging. std::atomic<int> currentFrame_ and std::atomic<bool> "
     "playing_ allow safe reads from the decode thread without a mutex."),
    ("QNetworkAccessManager (non-blocking HTTP)",
     "ApiClient uses Qt's built-in async HTTP stack. All uploads and fetches return immediately; "
     "results are delivered via Qt signals. This keeps the UI thread unblocked during potentially "
     "long video uploads. Upload progress (bytesSent / bytesTotal) feeds directly into a QProgressBar."),
    ("AppConfig via QSettings",
     "All tuneable parameters (API base URL, cache capacity, prefetch lookahead, theme) are persisted "
     "with QSettings and editable at runtime. This means the backend URL can be pointed at a remote "
     "server without recompiling — important for deployment flexibility."),
]

for title, desc in decisions:
    story.append(KeepTogether([
        h3(title),
        body(desc),
        sp(4),
    ]))

# ── 5. Python FastAPI Backend — API Contract ────────────────────────────────────
story.append(sp(6))
story.append(h1("5. Python FastAPI Backend — API Contract"))
story.append(hr())
story.append(body(
    "The C++ client communicates exclusively with a Python FastAPI service running at "
    "<b>http://localhost:8000</b> (configurable). The backend runs MediaPipe Pose on each "
    "video frame and computes biomechanical metrics. The following endpoints are consumed:"
))
story.append(sp(6))

api_rows = [
    ["Endpoint",                      "Method", "Description"],
    ["POST /analyze/upload",          "POST",   "Multipart upload of a video file. Returns full AnalysisResult JSON: pose_frames[], metrics{}, phases{}, top_matches[], recommendations[]."],
    ["GET  /database/pitchers",       "GET",    "Returns a JSON array of all PitcherProfile objects in the backend database (id, name, team, throwing_arm, photo_url, video_url)."],
    ["GET  /database/pitcher/{id}",   "GET",    "Returns a single PitcherProfile by pitcher ID."],
    ["GET  /health",                  "GET",    "Health check. Returns 200 OK if the backend is reachable. Used by the connection monitor (QTimer every 5 s)."],
]
story.append(header_table(api_rows, col_widths=[W*0.30, W*0.10, W*0.60]))

story.append(sp(10))
story.append(h2("5.1  /analyze/upload — Response JSON Schema"))
story.append(body("The JSON response is parsed by JsonParser::parseAnalysisResponse(). Key fields:"))
story.append(sp(4))

schema_rows = [
    ["JSON Field",                  "C++ Type",              "Notes"],
    ["pose_frames[].frame_number",  "int",                   "Video frame index"],
    ["pose_frames[].landmarks[]",   "PoseLandmark[33]",      "MediaPipe Pose — 33 keypoints: x, y (0–1 normalised), z, visibility (0–1)"],
    ["pose_frames[].width/height",  "int",                   "Source frame dimensions"],
    ["metrics.elbow_angle",         "double (°)",            "Shoulder–elbow–wrist angle at release"],
    ["metrics.hip_shoulder_sep",    "double (°)",            "Hip-to-shoulder rotational lag (kinetic chain)"],
    ["metrics.shoulder_rotation",   "double (°)",            "Shoulder rotation at release"],
    ["metrics.stride_length",       "double (×ht)",          "Ankle-to-ankle distance, body-height normalised"],
    ["metrics.delivery_tempo",      "double [0–1]",          "Release timing as fraction of total delivery"],
    ["metrics.release_height",      "double [0–1]",          "Vertical hand position at release, normalised"],
    ["metrics.arm_extension",       "double [0–1]",          "Forward reach from shoulder at release"],
    ["metrics.lead_knee_flexion",   "double (°)",            "Lead knee bend at foot plant"],
    ["phases.leg_lift / foot_plant\n/ max_arm_cock\n/ release_point\n/ follow_through",
                                    "int (frame index)",     "Key delivery phase frame indices → timeline markers"],
    ["top_matches[].pitcher_id",    "QString",               "Matched pro pitcher"],
    ["top_matches[].similarity",    "double [0–1]",          "Cosine similarity score"],
    ["top_matches[].coaching_cues", "QStringList",           "Natural-language coaching tips"],
    ["recommendations[]",           "QStringList",           "Overall improvement suggestions"],
]
story.append(header_table(schema_rows, col_widths=[W*0.28, W*0.22, W*0.50]))

# ── 6. Threading Model ─────────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("6. Threading Model"))
story.append(hr())

thread_rows = [
    ["Thread",         "Owner Class",        "What it does",                             "Cross-thread boundary"],
    ["UI Thread\n(main)",
                       "MainWindow,\nall Views,\nPresenters",
                       "Event loop, all widget painting, SessionModel writes",
                       "Receives signals from worker threads via QueuedConnection (automatic serialization)"],
    ["Decode Thread",  "VideoDecodeService", "Runs cv::VideoCapture seek+decode in a loop; blocks on FrameRequestQueue::popLatest()",
                       "Emits frameReady(QImage, int) → UI thread via QueuedConnection"],
    ["Analysis Thread","AnalysisThread",     "Drives ApiClient upload; receives HTTP response",
                       "Emits analysisComplete(AnalysisResult) → AnalysisPresenter on UI thread"],
    ["Network Thread\n(Qt internal)",
                       "QNetworkAccessManager",
                       "Async I/O for all HTTP requests",
                       "Qt manages internally; signals always delivered on the owning object's thread"],
]
story.append(header_table(thread_rows, col_widths=[W*0.18, W*0.20, W*0.32, W*0.30]))

story.append(sp(8))
story.append(body(
    "<b>Rule:</b> SessionModel writes only occur on the UI thread. "
    "std::atomic&lt;int&gt; currentFrame_ and std::atomic&lt;bool&gt; playing_ are the only members "
    "read from the decode thread — all other state is read/written exclusively on the UI thread."
))

# ── 7. Rendering Pipeline ──────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("7. Rendering Pipeline"))
story.append(hr())

story.append(h2("7.1  VideoPlayerWidget::paintEvent()"))
story.append(body("Each repaint executes the following layers in order:"))
steps = [
    "1. Fill widget background with black (QPainter::fillRect).",
    "2. Compute letterbox destination rect (aspect-ratio preserving, centred) → destRect_.",
    "3. Blit currentFrame_ (QImage) into destRect_ using drawImage().",
    "4. If poseVisible_ && currentPose_.isValid(): delegate to PoseOverlayRenderer::render().",
]
for s in steps:
    story.append(bullet(s))

story.append(sp(6))
story.append(h2("7.2  PoseOverlayRenderer::render() — 4 Layers"))
overlay_rows = [
    ["Layer",  "Method",              "What is drawn"],
    ["1",      "drawBones()",         "Lines between connected keypoint pairs (SkeletonTopology). Arm bones = orange, leg bones = light-blue, torso = cyan. Keypoints below minVisibility (0.5) are skipped."],
    ["2",      "drawJoints()",        "White filled circles at each visible keypoint (radius 4 px)."],
    ["3",      "drawAngles()",        "Angle annotations at elbow, shoulder, hip: semi-transparent black bg rectangle + white text showing computed degrees."],
    ["4",      "drawPhaseLabel()",    "Gold text in top-left corner: current PitchPhase string (e.g. 'ARM COCK')."],
]
story.append(header_table(overlay_rows, col_widths=[W*0.06, W*0.24, W*0.70]))

story.append(sp(6))
story.append(h2("7.3  MetricCard / IMetricRenderer — Strategy Pattern"))
story.append(body("Each MetricCard holds a unique_ptr<IMetricRenderer>. Three concrete implementations:"))
renderer_rows = [
    ["Class",                  "Metric type",         "Gauge style"],
    ["AngleMetricRenderer",    "Degrees (°)",         "Horizontal bar, colour from MetricThresholds: green (optimal) → amber (warning) → red (poor). Formatted as e.g. '87°'."],
    ["RatioMetricRenderer",    "Normalised [0–1]",    "Same horizontal bar. Formatted as e.g. '0.92'. Used for strideLength (×ht), armExtension, releaseHeight."],
    ["TempoMetricRenderer",    "Fraction [0–1]",      "Horizontal bar with mid-point marker. Represents deliveryTempo (release timing as fraction of full delivery)."],
]
story.append(header_table(renderer_rows, col_widths=[W*0.28, W*0.22, W*0.50]))

# ── 8. Key Utility Classes ─────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("8. Key Utility Classes"))
story.append(hr())

story.append(h2("8.1  FrameRequestQueue"))
story.append(body(
    "A thread-safe 'latest-only' queue. std::deque + std::mutex + std::condition_variable. "
    "push() (UI thread) appends and notifies. popLatest() (decode thread) waits on the condition variable, "
    "then drains the entire deque and returns only the last value. This guarantees that the decode thread "
    "never processes a request for a frame the user has already scrubbed past."
))

story.append(h2("8.2  FrameCache (LRU)"))
story.append(body(
    "Capacity-bounded cache mapping frame numbers to QImage. "
    "std::list<int> lruOrder_ maintains access order (front = MRU). "
    "std::unordered_map maps frame number → (QImage, list iterator) for O(1) lookup and O(1) eviction. "
    "QReadWriteLock permits concurrent reads (UI thread showing a frame while decode thread writes another)."
))

story.append(h2("8.3  JsonParser"))
story.append(body(
    "All methods are static — no instance state. Uses nlohmann/json internally. "
    "parseAnalysisResponse() is the primary entry point and assembles the full AnalysisResult from "
    "the /analyze/upload JSON body. Private helpers parsePoseFrame(), parseMetrics(), "
    "parsePhaseFrames(), parsePitcherMatch() each handle one object type."
))

story.append(h2("8.4  CoordMapper"))
story.append(body(
    "Converts MediaPipe's normalised landmark coordinates (x, y in [0,1] relative to frame size) "
    "to pixel coordinates within VideoPlayerWidget's letterboxed destRect. Called every paintEvent "
    "for each visible keypoint."
))

story.append(h2("8.5  MetricThresholds"))
story.append(body(
    "A namespace containing a static unordered_map of MetricRange objects keyed by metric name. "
    "MetricRange defines optimalMin/Max (green), warnMin/Max (amber), and displayMin/Max (gauge scale). "
    "colorFor(value) returns the appropriate QColor. normalize(value) maps value to [0,1] for bar width. "
    "Thresholds are grounded in baseball biomechanics literature."
))

# ── 9. Configuration ───────────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("9. Application Configuration (AppConfig)"))
story.append(hr())
story.append(body(
    "AppConfig is a plain struct loaded from and saved to QSettings at startup/shutdown. "
    "All parameters have sensible defaults that work out-of-the-box with a local backend."
))
story.append(sp(4))
cfg_rows = [
    ["Parameter",               "Default",   "Effect"],
    ["apiBaseUrl",              "http://localhost:8000", "FastAPI backend URL — change to point at a remote server"],
    ["frameCacheCapacity",      "60 frames", "LRU cache size — ~2 s at 30fps; reduce on low-RAM devices"],
    ["prefetchLookahead",       "30 frames", "PrefetchService decodes this many frames ahead during playback"],
    ["darkTheme",               "true",      "Loads dark_theme.qss stylesheet"],
    ["showAngleAnnotations",    "true",      "PoseOverlayRenderer.style.showAngles"],
    ["minKeypointVisibility",   "0.5",       "Keypoints with MediaPipe visibility < 0.5 are not rendered"],
]
story.append(header_table(cfg_rows, col_widths=[W*0.28, W*0.22, W*0.50]))

# ── 10. Test Coverage ──────────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("10. Test Coverage"))
story.append(hr())
test_rows = [
    ["Test file",                  "What is tested"],
    ["test_frame_cache.cpp",       "LRU eviction, thread-safe concurrent reads, capacity limits, contains() after evict"],
    ["test_json_parser.cpp",       "parseAnalysisResponse() against fixture JSON; missing fields; malformed input"],
    ["test_coord_mapper.cpp",      "Coordinate mapping from normalised landmark space → pixel space; edge cases (0,0), (1,1)"],
    ["test_frame_request_queue.cpp","push/popLatest() latest-only semantics; stop() unblocks blocked thread; concurrent push stress test"],
]
story.append(header_table(test_rows, col_widths=[W*0.35, W*0.65]))
story.append(sp(6))
story.append(body(
    "Tests use the CMake/CTest framework (add_subdirectory(tests)). "
    "Fixtures are in tests/fixtures/sample_analysis_response.json. "
    "The utility layer (FrameCache, JsonParser, CoordMapper, FrameRequestQueue) is fully tested "
    "without Qt dependencies where possible."
))

# ── 11. Directory Layout ───────────────────────────────────────────────────────
story.append(sp(10))
story.append(h1("11. Directory Layout"))
story.append(hr())
dir_rows = [
    ["Path",                    "Contents"],
    ["src/app/",                "Application, MainWindow, AppConfig — entry point and window shell"],
    ["src/models/",             "SessionModel, AnalysisResult, PoseFrame, BiomechanicsMetrics, FrameCache, PitcherProfile"],
    ["src/views/",              "VideoPlayerWidget, PoseOverlayRenderer, TimelineWidget, MetricsPanel, MetricCard, ComparisonView, PitcherMatchPanel"],
    ["src/presenters/",         "MainPresenter, VideoPresenter, AnalysisPresenter, ComparisonPresenter"],
    ["src/services/",           "VideoDecodeService, ApiClient, AnalysisThread, ThumbnailGenerator, PrefetchService"],
    ["src/rendering/",          "IMetricRenderer, AngleMetricRenderer, RatioMetricRenderer, TempoMetricRenderer"],
    ["src/utils/",              "FrameRequestQueue, JsonParser, CoordMapper, SkeletonTopology, MetricThresholds"],
    ["resources/",              "dark_theme.qss, resources.qrc"],
    ["tests/",                  "Unit tests + fixtures"],
    ["third_party/",            "nlohmann/json (header-only)"],
    ["CMakeLists.txt",          "Root build; CMakePresets.json defines debug/release presets (Ninja generator)"],
]
story.append(header_table(dir_rows, col_widths=[W*0.30, W*0.70]))

# ── Footer note ────────────────────────────────────────────────────────────────
story.append(sp(16))
story.append(hr(C_ACCENT, 1))
story.append(Paragraph(
    "Interactive Pitch Analysis Viewer  ·  C++ Portfolio Project  ·  2026",
    style("footer", fontSize=8, textColor=C_GRAY, alignment=TA_CENTER)
))

# ── Build ──────────────────────────────────────────────────────────────────────
doc.build(story)
print(f"PDF written → {OUTPUT}")
