# cmake/freertos.cmake
include(FetchContent)

FetchContent_Declare(
    freertos
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
    # GIT_TAG        V11.3.0
    GIT_TAG        V10.4.3


)
FetchContent_Populate(freertos)

add_library(freertos_kernel STATIC
    ${freertos_SOURCE_DIR}/tasks.c
    ${freertos_SOURCE_DIR}/queue.c
    ${freertos_SOURCE_DIR}/list.c
    ${freertos_SOURCE_DIR}/timers.c
    ${freertos_SOURCE_DIR}/event_groups.c
    ${freertos_SOURCE_DIR}/stream_buffer.c
    ${freertos_SOURCE_DIR}/portable/MemMang/heap_4.c
)

target_sources(freertos_kernel PRIVATE
    ${CMAKE_SOURCE_DIR}/platform/x86/port/port.c
    ${CMAKE_SOURCE_DIR}/platform/x86/port/portasm.S
)

target_include_directories(freertos_kernel PUBLIC
    ${freertos_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
)

zelix_apply_common_compile_options(freertos_kernel)
