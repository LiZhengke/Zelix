add_library(loader STATIC
    src/loader.c
)

target_include_directories(loader PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/kernel/fatfs/inc
    ${CMAKE_SOURCE_DIR}/kernel/loader/inc
    ${CMAKE_SOURCE_DIR}/kernel/mm/inc
    ${fatfs_SOURCE_DIR}/source
)

zelix_apply_common_compile_options(loader)

target_link_libraries(loader PUBLIC
    freertos_kernel
)
