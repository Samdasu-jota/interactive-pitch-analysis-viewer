cmake_minimum_required(VERSION 3.22)
project(InteractivePitchViewer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS
    Core Widgets Network Concurrent
)

find_package(OpenCV REQUIRED COMPONENTS core videoio imgproc)

# nlohmann/json — bundled header-only
add_subdirectory(third_party)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp

    # app
    src/app/Application.cpp
    src/app/MainWindow.cpp
    src/app/AppConfig.cpp

    # models
    src/models/AnalysisResult.cpp
    src/models/SessionModel.cpp
    src/models/FrameCache.cpp

    # views
    src/views/VideoPlayerWidget.cpp
    src/views/PoseOverlayRenderer.cpp
    src/views/TimelineWidget.cpp
    src/views/MetricsPanel.cpp
    src/views/MetricCard.cpp
    src/views/ComparisonView.cpp
    src/views/PitcherMatchPanel.cpp
    src/views/SpeedControlWidget.cpp

    # presenters
    src/presenters/MainPresenter.cpp
    src/presenters/VideoPresenter.cpp
    src/presenters/AnalysisPresenter.cpp
    src/presenters/ComparisonPresenter.cpp

    # services
    src/services/VideoDecodeService.cpp
    src/services/ApiClient.cpp
    src/services/AnalysisThread.cpp
    src/services/ThumbnailGenerator.cpp
    src/services/PrefetchService.cpp

    # utils
    src/utils/FrameRequestQueue.cpp
    src/utils/JsonParser.cpp
    src/utils/CoordMapper.cpp

    # rendering
    src/rendering/AngleMetricRenderer.cpp
    src/rendering/RatioMetricRenderer.cpp
    src/rendering/TempoMetricRenderer.cpp

    # Qt resources
    resources/resources.qrc
)

set(HEADERS
    src/app/Application.h
    src/app/MainWindow.h
    src/app/AppConfig.h

    src/models/PoseLandmark.h
    src/models/PoseFrame.h
    src/models/BiomechanicsMetrics.h
    src/models/AnalysisResult.h
    src/models/PitcherProfile.h
    src/models/SessionModel.h
    src/models/FrameCache.h

    src/views/VideoPlayerWidget.h
    src/views/PoseOverlayRenderer.h
    src/views/TimelineWidget.h
    src/views/MetricsPanel.h
    src/views/MetricCard.h
    src/views/ComparisonView.h
    src/views/PitcherMatchPanel.h
    src/views/SpeedControlWidget.h

    src/presenters/MainPresenter.h
    src/presenters/VideoPresenter.h
    src/presenters/AnalysisPresenter.h
    src/presenters/ComparisonPresenter.h

    src/services/VideoDecodeService.h
    src/services/ApiClient.h
    src/services/AnalysisThread.h
    src/services/ThumbnailGenerator.h
    src/services/PrefetchService.h

    src/utils/FrameRequestQueue.h
    src/utils/JsonParser.h
    src/utils/SkeletonTopology.h
    src/utils/CoordMapper.h
    src/utils/MetricThresholds.h

    src/rendering/IMetricRenderer.h
    src/rendering/AngleMetricRenderer.h
    src/rendering/RatioMetricRenderer.h
    src/rendering/TempoMetricRenderer.h
)

# ── Main Executable ───────────────────────────────────────────────────────────
add_executable(InteractivePitchViewer ${SOURCES} ${HEADERS})

target_include_directories(InteractivePitchViewer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(InteractivePitchViewer PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Concurrent
    ${OpenCV_LIBS}
    nlohmann_json::nlohmann_json
)

# Compiler warnings
target_compile_options(InteractivePitchViewer PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:
        -Wall -Wextra -Wpedantic -Wno-unused-parameter>
)

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()
add_subdirectory(tests)
