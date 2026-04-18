add_library(task STATIC
    src/ktask.c
)

target_include_directories(task PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/libc/inc
    ${CMAKE_SOURCE_DIR}/kernel/sched/inc
    ${CMAKE_SOURCE_DIR}/kernel/task/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(task)

target_link_libraries(task PRIVATE
    sched
    freertos_kernel
)
