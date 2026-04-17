# Memory management subsystem placeholder.
add_library(mm STATIC
    src/mmu.c
    src/pmm.c
    src/vmm.c
    src/heap_alloc.c
)

target_include_directories(mm PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/libc/inc
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(mm)
