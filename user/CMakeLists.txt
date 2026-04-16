# User-space build entry point.
set(USER_SUBDIRS
    libc
    bin
    linker
)

foreach(subdir IN LISTS USER_SUBDIRS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
        add_subdirectory(${subdir})
    endif()
endforeach()
