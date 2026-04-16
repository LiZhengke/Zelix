# Platform-specific build entry point.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/x86/CMakeLists.txt")
    add_subdirectory(x86)
endif()
