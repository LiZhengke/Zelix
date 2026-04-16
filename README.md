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

## Build System

The project uses CMake with a top-level configuration that conditionally includes
subdirectories when their `CMakeLists.txt` files are present.

### Configure

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build
```

## Current Status

This repository currently provides the project structure and build-system
scaffolding. Source files and concrete build targets can be added incrementally
inside each module.
