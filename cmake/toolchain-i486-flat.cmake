set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i486)

set(CMAKE_C_COMPILER gcc CACHE STRING "C compiler for the i486 flat target")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}" CACHE STRING "ASM compiler for the i486 flat target")
set(CMAKE_LINKER ld CACHE STRING "Linker for the i486 flat target")
set(CMAKE_OBJCOPY objcopy CACHE STRING "objcopy for the i486 flat target")

set(CMAKE_C_FLAGS_INIT "-m32 -ffreestanding -fno-pic -fno-pie -fno-stack-protector -fno-builtin")
set(CMAKE_ASM_FLAGS_INIT "-m32 -fno-pic -fno-pie")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-m32 -no-pie")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
