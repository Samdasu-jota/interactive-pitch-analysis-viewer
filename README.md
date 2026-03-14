# Interactive Pitch Analysis Viewer

A **C++ Qt6 desktop application** that visualizes baseball pitching biomechanics from AI-driven pose analysis. Built as a portfolio project demonstrating the C++ software engineering skills used at Tesla's Vehicle UI / Infotainment team.

---

## Architecture

The application follows a strict **MVP (Model-View-Presenter)** pattern over a **5-thread producer-consumer pipeline**:

```
UI Thread          VideoDecodeThread    AnalysisThread    PrefetchThread    ThumbnailThread
─────────          ─────────────────    ──────────────    ──────────────    ───────────────
Qt event loop      cv::VideoCapture     HTTP upload       Frame lookahead   1fps thumbnails
paintEvent()       FrameRequestQueue    JSON parsing      PrefetchService   QtConcurrent
User input         LRU FrameCache       Emits result      std::thread       ThumbnailGen
```

**Key architectural decisions:**

- **Latest-only `FrameRequestQueue`**: During rapid timeline scrubbing, the decode thread drains the entire queue and processes only the newest request — eliminating latency accumulation, identical to how vehicle sensor UI handles rapidly-changing values.
- **Zero-copy `QImage` from `cv::Mat`**: Frames are wrapped in `QImage` pointing to OpenCV's buffer without copying on the render path.
- **`QReadWriteLock` on `FrameCache`**: Allows concurrent reads (UI renders frame N while decode writes frame N+5).
- **`Qt::QueuedConnection`** everywhere cross-thread — no shared mutable state between threads without explicit synchronization.

---

## Features

- Load and play pitching videos (MP4, MOV, AVI)
- Frame-accurate timeline scrubbing with thumbnail strip
- **33-keypoint MediaPipe skeleton overlay** drawn via QPainter
- **14 biomechanical metrics** with animated gauge bars (300ms QPropertyAnimation)
- Phase event markers (leg lift, foot plant, release point) on timeline
- Jump-to-phase buttons (`1` / `2` / `3` keyboard shortcuts)
- **Side-by-side comparison** with synced playback against pro pitchers
- Top-3 pitcher similarity matches with coaching cues
- Dark theme (Tesla-adjacent aesthetic)
- Drag-and-drop video loading
- Export annotated frame as PNG

---

## Tech Stack

| Component | Technology |
|---|---|
| Language | C++17 |
| UI Framework | Qt 6 Widgets |
| Video decoding | OpenCV 4.x (`cv::VideoCapture`) |
| JSON parsing | nlohmann/json (header-only, bundled) |
| HTTP client | `QNetworkAccessManager` |
| Threading | `QThread`, `std::thread`, `QtConcurrent` |
| Build system | CMake 3.22+ with Presets |
| Tests | Google Test (fetched via CMake FetchContent) |
| AI backend | Python FastAPI (separate repo) |

---

## Prerequisites

```bash
# macOS (Homebrew)
brew install qt@6 opencv cmake ninja

# Ubuntu
sudo apt install qt6-base-dev libopencv-dev cmake ninja-build
```

---

## Build

```bash
# Configure (debug)
cmake --preset debug

# Build
cmake --build --preset debug

# Run
./build/debug/InteractivePitchViewer
```

---

## Running with the AI Backend

The application connects to the Python FastAPI backend at `http://localhost:8000`.

```bash
# In the baseball-pitch-recommendation-ai repo:
pip install -r requirements.txt
uvicorn src.api:app --reload

# Then launch the C++ app — the connection indicator turns green
```

---

## Project Structure

```
src/
├── app/          Application, MainWindow, AppConfig
├── models/       PoseFrame, AnalysisResult, SessionModel, FrameCache
├── views/        VideoPlayerWidget, TimelineWidget, MetricsPanel, ComparisonView
├── presenters/   MainPresenter, VideoPresenter, AnalysisPresenter
├── services/     VideoDecodeService, ApiClient, AnalysisThread, ThumbnailGenerator
├── utils/        FrameRequestQueue, JsonParser, CoordMapper, SkeletonTopology
└── rendering/    IMetricRenderer, AngleMetricRenderer, RatioMetricRenderer
tests/            Google Test unit tests + JSON fixtures
resources/        Dark theme QSS, icons, resource bundle
```

---

## Tests

```bash
cd build/debug && ctest --output-on-failure
```

Tests cover:
- `FrameCache` — LRU eviction, thread-safety under concurrent read/write
- `JsonParser` — full API response parsing, edge cases
- `CoordMapper` — letterbox rect geometry, keypoint coordinate mapping
- `FrameRequestQueue` — latest-only behavior, stop signal, concurrent push/pop

---

## Keyboard Shortcuts

| Key | Action |
|---|---|
| `Space` | Play / Pause |
| `←` / `→` | Step frame backward / forward |
| `1` | Jump to Leg Lift |
| `2` | Jump to Foot Plant |
| `3` | Jump to Release Point |
| `Ctrl+O` | Open video |
| `Ctrl+E` | Export annotated frame |

---

## Resume Bullet Points

> Built a multithreaded C++17 Qt6 desktop application with a 5-thread producer-consumer pipeline decoupling video decode, pose data streaming, and UI rendering; eliminated scrubbing latency using a latest-only lock-free frame request queue.

> Designed a custom OpenCV + QPainter rendering pipeline compositing 33-keypoint MediaPipe skeleton overlays onto live video frames at 60fps, using zero-copy QImage construction from cv::Mat buffers to eliminate memory allocation on the critical rendering path.

> Architected a strict MVP-layered application with 5 isolated execution contexts communicating exclusively via Qt queued signals and std::atomic status flags — zero shared mutable state without explicit synchronization.

> Integrated a real-time biomechanics analysis API via asynchronous QNetworkAccessManager HTTP client, parsing 14 joint-angle and tempo metrics from MediaPipe pose JSON into animated metric gauge visualizations with QPropertyAnimation-driven transitions.

> Implemented a frame-accurate timeline widget with thumbnail strip, keyboard-driven playback, phase-event jump buttons, and a synchronized side-by-side comparison view between user and pro pitcher mechanics, demonstrating production-quality interactive C++ UI engineering.
