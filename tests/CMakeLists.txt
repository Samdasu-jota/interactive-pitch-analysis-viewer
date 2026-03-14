include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# ── Test helper library (shared model/utility code) ───────────────────────────
add_library(test_helpers STATIC
    ${CMAKE_SOURCE_DIR}/src/models/AnalysisResult.cpp
    ${CMAKE_SOURCE_DIR}/src/models/FrameCache.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/FrameRequestQueue.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/JsonParser.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/CoordMapper.cpp
)
target_include_directories(test_helpers PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(test_helpers PUBLIC
    Qt6::Core
    nlohmann_json::nlohmann_json
)

# ── test_frame_cache ──────────────────────────────────────────────────────────
add_executable(test_frame_cache test_frame_cache.cpp)
target_link_libraries(test_frame_cache PRIVATE test_helpers GTest::gtest_main Qt6::Gui)
gtest_discover_tests(test_frame_cache)

# ── test_json_parser ──────────────────────────────────────────────────────────
add_executable(test_json_parser test_json_parser.cpp)
target_link_libraries(test_json_parser PRIVATE test_helpers GTest::gtest_main)
gtest_discover_tests(test_json_parser)

# ── test_coord_mapper ─────────────────────────────────────────────────────────
add_executable(test_coord_mapper test_coord_mapper.cpp)
target_link_libraries(test_coord_mapper PRIVATE test_helpers GTest::gtest_main Qt6::Gui)
gtest_discover_tests(test_coord_mapper)

# ── test_frame_request_queue ──────────────────────────────────────────────────
add_executable(test_frame_request_queue test_frame_request_queue.cpp)
target_link_libraries(test_frame_request_queue PRIVATE test_helpers GTest::gtest_main)
gtest_discover_tests(test_frame_request_queue)
