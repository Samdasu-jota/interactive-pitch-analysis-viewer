# nlohmann/json — header-only, bundled
# If not present locally, download via FetchContent
include(FetchContent)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann/json.hpp")
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann"
    )
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
    message(STATUS "nlohmann/json: using bundled header")
else()
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
        DOWNLOAD_NO_EXTRACT TRUE
        DOWNLOAD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann"
    )
    FetchContent_MakeAvailable(nlohmann_json)
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/nlohmann"
    )
    add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
endif()
