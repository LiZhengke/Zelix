add_library(sched STATIC
    src/adapter.c
)

target_include_directories(sched PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/sched/inc
    ${CMAKE_SOURCE_DIR}/kernel/task/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(sched)

target_link_libraries(sched PRIVATE
    freertos_kernel
)
