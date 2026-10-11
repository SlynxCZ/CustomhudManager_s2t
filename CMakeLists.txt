cmake_minimum_required(VERSION 3.18)
project(customhud_manager CXX)

set(CMAKE_CXX_STANDARD 20)

# The SDK library is built against the static CRT (/MT); a plugin linking the
# dynamic one gets unresolved CRT internals (_recalloc_base from the hl2sdk's
# memoverride.cpp). Set before the SDK is added, so both agree.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Source files are UTF-8 (the author strings); MSVC would read them as the ANSI code page.
if (MSVC)
    add_compile_options(/utf-8)
endif ()

# The SDK is resolved from the environment, the same way source2toolkit core
# resolves it -- one checkout for the toolkit and everything built against it.
if (DEFINED ENV{SOURCE2TOOLKIT_SDK} AND NOT "$ENV{SOURCE2TOOLKIT_SDK}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{SOURCE2TOOLKIT_SDK}" SOURCE2TOOLKIT_SDK)
else ()
    message(FATAL_ERROR "SOURCE2TOOLKIT_SDK env variable not set")
endif ()

add_subdirectory(${SOURCE2TOOLKIT_SDK} ${CMAKE_BINARY_DIR}/source2toolkit-sdk)

include(makefiles/cmake/shared.cmake)

if (UNIX AND NOT APPLE)
    include(makefiles/cmake/linux.cmake)
elseif (WIN32)
    include(makefiles/cmake/windows.cmake)
endif ()

# The plugin binary (makefiles/ambuild/AMBuilder on the AMBuild side).
include(${CMAKE_SOURCE_DIR}/makefiles/cmake/CMakeBuilder.cmake)

# Install layout (makefiles/ambuild/package.ambuild on the AMBuild side).
include(makefiles/cmake/package.cmake)
