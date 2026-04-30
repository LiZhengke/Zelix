include(FetchContent)

FetchContent_Declare(
  fatfs
  GIT_REPOSITORY https://github.com/abbrev/fatfs.git
  GIT_TAG        R0.16
)

FetchContent_Populate(fatfs)

# Override the bundled ffconf.h with the project-specific one.
configure_file(
    ${CMAKE_SOURCE_DIR}/kernel/fs/fatfs/inc/ffconf.h
    ${fatfs_SOURCE_DIR}/source/ffconf.h
    COPYONLY
)

add_library(fatfs STATIC
    ${fatfs_SOURCE_DIR}/source/ff.c
    ${fatfs_SOURCE_DIR}/source/ffsystem.c
    ${fatfs_SOURCE_DIR}/source/ffunicode.c
    ${CMAKE_SOURCE_DIR}/kernel/fs/fatfs/src/diskio.c
    ${CMAKE_SOURCE_DIR}/kernel/fs/fatfs/src/fatfs.c
    ${CMAKE_SOURCE_DIR}/kernel/dev/blk/src/ata.c
)

target_include_directories(fatfs PUBLIC
    ${CMAKE_SOURCE_DIR}/kernel/fs/fatfs/inc
    ${CMAKE_SOURCE_DIR}/kernel/dev/blk/inc
    ${CMAKE_SOURCE_DIR}/kernel/include
    ${CMAKE_SOURCE_DIR}/platform/x86/inc
    PRIVATE
    ${fatfs_SOURCE_DIR}/source
)

target_compile_options(fatfs PRIVATE
    -Wno-unused-parameter
    -Wno-unused-variable
)
