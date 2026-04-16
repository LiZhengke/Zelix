# Kernel build entry point.
message("PROJECT_NAME: ${PROJECT_NAME}")

set(KERNEL_SUBDIRS
    proc
    thread
    syscall
    mm
    fs
    sched
)

foreach(subdir IN LISTS KERNEL_SUBDIRS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
        add_subdirectory(${subdir})
    endif()
endforeach()
add_executable(${PROJECT_NAME}
    main.c
    # task/task.c
    # mm/mm.c
    # syscall/sys_exit.c
)

