cmake_minimum_required(VERSION 3.18)
project(customhud_manager CXX)

set(CMAKE_CXX_STANDARD 20)

# The SDK is resolved from the environment, the same way source2toolkit core
# resolves it -- one checkout for the toolkit and everything built against it.
if (DEFINED ENV{SOURCE2TOOLKIT_SDK} AND NOT "$ENV{SOURCE2TOOLKIT_SDK}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{SOURCE2TOOLKIT_SDK}" SOURCE2TOOLKIT_SDK)
else ()
    message(FATAL_ERROR "SOURCE2TOOLKIT_SDK env variable not set")
endif ()

set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/addons/source2toolkit/plugins")

add_subdirectory(${SOURCE2TOOLKIT_SDK} ${CMAKE_BINARY_DIR}/source2toolkit-sdk)

# The version the plugin reports is the release tag and the commit: SEMVER and
# GITHUB_SHA_SHORT come from the CI, a local build says "Local".
if (DEFINED ENV{GITHUB_SHA_SHORT} AND NOT "$ENV{GITHUB_SHA_SHORT}" STREQUAL "")
    add_definitions(-DGITHUB_SHA="$ENV{GITHUB_SHA_SHORT}")
else ()
    find_package(Git QUIET)
    set(GIT_SHA "Local")
    if (GIT_FOUND)
        execute_process(
                COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE GIT_SHA
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
        )
    endif ()
    add_definitions(-DGITHUB_SHA="${GIT_SHA}")
endif ()

if (DEFINED ENV{SEMVER} AND NOT "$ENV{SEMVER}" STREQUAL "")
    add_definitions(-DSEMVER="$ENV{SEMVER}")
else ()
    add_definitions(-DSEMVER="Local")
endif ()

file(GLOB_RECURSE SOURCE_FILES
        src/*.cpp
        src/*.h
        src/*.hpp
)

add_s2toolkit_plugin(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
        # The .dll on Windows is a RUNTIME artifact.
        RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)
