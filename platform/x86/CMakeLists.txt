# x86 platform build placeholder.
add_library(x86_platform STATIC
    src/start.S
    src/tss.c
    src/page.c
)

target_include_directories(x86_platform PRIVATE
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

target_compile_options(x86_platform PRIVATE
    -ffreestanding
    -fno-builtin
    -fno-stack-protector
    -fno-pic
    -m32
    -std=gnu11
)

target_link_options(${PROJECT_NAME} PRIVATE
	-nostdlib
    -nostartfiles
	-m32
    -no-pie
)
