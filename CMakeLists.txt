cmake_minimum_required(VERSION 3.20)
include(cmake/freertos.cmake)

project(Zelix
	VERSION 0.1.0
	LANGUAGES C ASM
)

# Common defaults for C targets in this workspace.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Keep reusable cmake modules in the local cmake/ directory.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Add directories only when they define their own CMakeLists.txt.
# This keeps configuration successful while the tree is being bootstrapped.
set(ZELIX_SUBDIRS
	kernel
	user
	platform
	third_party
)

foreach(subdir IN LISTS ZELIX_SUBDIRS)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/CMakeLists.txt")
		add_subdirectory(${subdir})
	endif()
endforeach()

target_compile_options(${PROJECT_NAME} PRIVATE
    -ffreestanding
    -fno-builtin
	-fno-stack-protector
	-fno-pic
    -m32
    -std=gnu11
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    freertos_kernel
	x86_platform
	mm
)

target_link_options(${PROJECT_NAME} PRIVATE
	-nostdlib
    -nostartfiles
	-m32
	-Wl,--build-id=none
    -T ${CMAKE_SOURCE_DIR}/platform/x86/linker.ld
    -Wl,-Map=${PROJECT_NAME}.map
    -Wl,--cref
)
