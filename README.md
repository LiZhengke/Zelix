# Zelix

Zelix is an experimental i486 OS kernel project using FreeRTOS as the core
scheduler, with an x86 platform port, kernel-side libc, and modular kernel
subsystems.

## Repository Layout

```text
Zelix/
├── kernel/
│   ├── fs/
│   ├── libc/
│   ├── mm/
│   ├── sched/
│   ├── syscall/
│   ├── task/
│   ├── gdbcmds
│   ├── main.c
│   └── CMakeLists.txt
├── platform/
│   └── x86/
│       ├── inc/
│       ├── port/
│       ├── src/
│       ├── linker.ld
│       └── CMakeLists.txt
├── user/
│   ├── libc/
│   ├── bin/
│   ├── linker/
│   └── CMakeLists.txt
├── third_party/
├── include/
├── cmake/
└── CMakeLists.txt
```

## Build (i486)

Configure:

```bash
cmake -S . -B build-i486 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-i486-flat.cmake
```

Build:

```bash
cmake --build build-i486
```

Clean rebuild:

```bash
cmake --build build-i486 --clean-first
```

If `build-i486/` was created with the wrong toolchain settings:

```bash
rm -rf build-i486
cmake -S . -B build-i486 -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-i486-flat.cmake
cmake --build build-i486
```

## Quick Run (QEMU)

Example run command:

```bash
/opt/qemu-7.2/bin/qemu-system-i386 \
  -cpu 486 \
  -m 128M \
  -bios /home/max/work/freertos/FreeRTOS-Kernel/i486-flat/bootloader/build/bios.rom \
  -kernel build-i486/kernel/Zelix \
  -nographic
```

Timed run for log capture:

```bash
timeout 12 /opt/qemu-7.2/bin/qemu-system-i386 -cpu 486 -m 128M \
  -bios /home/max/work/freertos/FreeRTOS-Kernel/i486-flat/bootloader/build/bios.rom \
  -kernel build-i486/kernel/Zelix -nographic 2>&1
```

## Output Artifacts

```text
build-i486/kernel/Zelix
build-i486/kernel/Zelix.map
```

## For Contributors

- Start with this README for build/run basics.
- See [kernel/DEVELOPMENT.md](kernel/DEVELOPMENT.md) for port internals,
  context-switch details, interrupt flow, and troubleshooting.
