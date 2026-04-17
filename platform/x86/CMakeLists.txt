# x86 platform build placeholder.
add_library(x86_platform STATIC
    src/start.S
    src/tss.c
    src/page.c
    src/gdt.c
    src/idt.c
    src/exception.S
    src/interrupt.c
)

target_include_directories(x86_platform PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/libc/inc
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(x86_platform)

target_link_libraries(x86_platform PRIVATE
    freertos_kernel
)
