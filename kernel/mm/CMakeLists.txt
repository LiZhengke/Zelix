# Memory management subsystem placeholder.
add_library(mm STATIC
    src/mmu.c
    src/pmm.c
    src/vmm.c
    src/heap_alloc.c
    src/mm.c
)

target_include_directories(mm PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/libc/inc
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${CMAKE_SOURCE_DIR}/kernel/task/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
    ${CMAKE_SOURCE_DIR}/kernel/sched/inc
    ${CMAKE_SOURCE_DIR}/kernel/loader/inc
)

zelix_apply_common_compile_options(mm)

target_link_libraries(mm PRIVATE
    freertos_kernel
    sched
)
