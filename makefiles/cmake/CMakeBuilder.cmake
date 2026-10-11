# The plugin binary -- CMake twin of makefiles/ambuild/AMBuilder: the plugin's
# own sources in AMBuilder's order, its include dirs and what it links with
# (the toolkit SDK's add_s2toolkit_plugin() adds the SDK side).
# Included by CMakeLists.txt after shared.cmake and the platform file.

# The plugin's own sources -- the same list, in the same order, as
# makefiles/ambuild/AMBuilder. Listed, not globbed: a new .cpp goes into both.
set(SOURCE_FILES
        src/plugin.cpp
        src/hud.cpp
        src/hudmenu.cpp
)

add_s2toolkit_plugin(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        # ICustomhudManager.h: the interface other plugins include.
        ${CMAKE_CURRENT_SOURCE_DIR}/public
)

if (LINK_LIBRARIES)
    target_link_libraries(${PROJECT_NAME} ${LINK_LIBRARIES})
endif ()

# What the platform file asked for on the link line (the export map on Linux),
# like linux.ambuild / windows.ambuild's ConfigureBinary() on the AMBuild side.
if (PLUGIN_LINK_OPTIONS)
    target_link_options(${PROJECT_NAME} PRIVATE ${PLUGIN_LINK_OPTIONS})
endif ()
if (PLUGIN_LINK_DEPENDS)
    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_DEPENDS ${PLUGIN_LINK_DEPENDS})
endif ()
