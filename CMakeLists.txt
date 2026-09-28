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

# plugin-metadata.json is the one place the plugin's name, author and version
# live. tools/version_gen.py turns it into version_gen.h, and the AMBuild build
# calls the same script, so the two cannot drift.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(VERSION_GEN_DIR "${CMAKE_BINARY_DIR}/versioning")

execute_process(
        COMMAND ${Python3_EXECUTABLE}
                ${CMAKE_CURRENT_SOURCE_DIR}/tools/version_gen.py
                --metadata ${CMAKE_CURRENT_SOURCE_DIR}/plugin-metadata.json
                --output ${VERSION_GEN_DIR}/version_gen.h
        RESULT_VARIABLE VERSION_GEN_RESULT
)

if (NOT VERSION_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "version_gen.py failed (${VERSION_GEN_RESULT})")
endif ()

# Re-run cmake when the metadata changes.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
        ${CMAKE_CURRENT_SOURCE_DIR}/plugin-metadata.json)

file(GLOB_RECURSE SOURCE_FILES
        src/*.cpp
        src/*.h
        src/*.hpp
)

add_s2toolkit_plugin(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${VERSION_GEN_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
        # The .dll on Windows is a RUNTIME artifact.
        RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)
