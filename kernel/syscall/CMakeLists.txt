# Syscall subsystem placeholder.
# Memory management subsystem placeholder.
add_library(syscall STATIC
    src/syscall.c
)

target_include_directories(syscall PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/syscall/inc
    ${CMAKE_SOURCE_DIR}/kernel/sched/inc
    ${CMAKE_SOURCE_DIR}/kernel/task/inc
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${CMAKE_SOURCE_DIR}/kernel/loader/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(syscall)

target_link_libraries(syscall PRIVATE
    freertos_kernel
    sched
)
