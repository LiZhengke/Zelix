# Memory management subsystem placeholder.
add_library(mm STATIC
    src/mmu.c
    src/pmm.c
    src/vmm.c
    src/gdt.c
    src/heap_alloc.c
)

target_include_directories(mm PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

target_compile_options(mm PRIVATE
    -ffreestanding
    -fno-builtin
    -fno-stack-protector
    -fno-pic
    -m32
    -std=gnu11
)

target_link_options(mm PRIVATE
    -nostdlib
    -nostartfiles
    -m32
    -no-pie
)
