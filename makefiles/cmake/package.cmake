# The install layout -- CMake twin of makefiles/ambuild/package.ambuild: a
# toolkit plugin drops into addons/source2toolkit/plugins, no metamod .vdf.
set(PLUGIN_OUTPUT_DIR "${CMAKE_BINARY_DIR}/addons/source2toolkit/plugins")

set_target_properties(${PROJECT_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
        # The .dll on Windows is a RUNTIME artifact.
        RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}
)
