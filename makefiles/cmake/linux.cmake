# Linux settings -- CMake twin of makefiles/ambuild/linux.ambuild. The compiler
# flags come from the SDK's add_s2toolkit_plugin().

# Export only CreateInterface -- the same version script AMBuild links with
# (makefiles/ambuild/linux.ambuild).
set(PLUGIN_EXPORT_MAP ${CMAKE_CURRENT_LIST_DIR}/../ExportMap)
list(APPEND PLUGIN_LINK_OPTIONS "-Wl,--version-script=${PLUGIN_EXPORT_MAP}")
list(APPEND PLUGIN_LINK_DEPENDS ${PLUGIN_EXPORT_MAP})
