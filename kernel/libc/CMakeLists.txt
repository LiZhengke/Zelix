# Kernel libc components.
add_library(kernel_libc STATIC
    src/printf.c
    src/putchar.c
    src/string.c
)

target_include_directories(kernel_libc PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/libc/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(kernel_libc)
