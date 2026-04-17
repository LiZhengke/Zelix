# Zelix

Zelix is an operating system project scaffold with a modular kernel, user-space
components, and platform-specific code.

## Repository Layout

```text
Zelix/
├── kernel/
│   ├── proc/
│   ├── thread/
│   ├── syscall/
│   ├── mm/
│   ├── fs/
│   ├── sched/
│   ├── include/
│   └── CMakeLists.txt
├── user/
│   ├── libc/
│   ├── bin/
│   ├── include/
│   ├── linker/
│   └── CMakeLists.txt
├── platform/
│   ├── x86/
│   └── CMakeLists.txt
├── include/
│   └── zelix/
├── third_party/
│   └── CMakeLists.txt
├── cmake/
└── CMakeLists.txt
```

## Build

The project uses CMake. For the i486 flat-memory kernel build, configure with
the repository toolchain file and use a dedicated build directory.

### Configure an i486 Build

```bash
cmake -S . -B build-i486 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-i486-flat.cmake
```

### Build

```bash
cmake --build build-i486
```

### Reconfigure from Scratch

If `build-i486/` was created without the toolchain file, remove it and
configure again:

```bash
rm -rf build-i486
cmake -S . -B build-i486 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-i486-flat.cmake
```

### Generated Artifacts

After a successful build, the main kernel executable and map file are written to:

```text
build-i486/kernel/Zelix
build-i486/kernel/Zelix.map
```

## Notes

- The first configure step fetches the FreeRTOS kernel with CMake `FetchContent`,
  so network access is required unless the dependency is already available
  locally.
- The toolchain file sets platform-level defaults for the i486 target. Project
  compile defaults shared by multiple targets are centralized in
  `cmake/zelix-target-defaults.cmake`.

## Current Status

The repository now contains a bootable kernel-oriented build with platform
support under `platform/x86`, kernel-side libc code under `kernel/libc`, and
memory-management code under `kernel/mm`. The tree is still evolving, but the
main i486 CMake build is active and usable.
