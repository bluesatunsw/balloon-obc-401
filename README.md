# balloon-obc-401
A flexible on-board-computer for high altitude balloons based on the dual core STM32H7 series running FreeRTOS.

## Getting Started
### Dependencies
Before starting you require the following:
- [ARM Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
- [CMake](https://cmake.org/download/)
- [Clang](https://releases.llvm.org/download.html)
- [STLINK](https://github.com/stlink-org/stlink)
- [STM32CubeMX](https://www.st.com/content/st_com/en/stm32cubemx.html)

```sh
# Arch Installation
sudo pacman -Syu gcc-arm-none-eabi gcc-arm-none-eabi-newlib clang cmake stlink
paru -S stm32cubemx
```

This repository uses Git submodules for software libraries. The submodules can be downloaded with:
```sh
git submodule update --init --remote --recursive
```

### Building
All commands should be run from the root directory of the project. To build everything run:
```sh
cmake -DCMAKE_C_COMPILER=[C_COMPILER] -DCMAKE_CXX_COMPILER=[CPP_COMPILER] -Bbuild .
cmake --build build --config Debug --target obc_m7
```
`[C_COMPILER]` and `[CPP_COMPILER]` must be replaced with with the compiler which corresponds to the hardware being targetted. For local testing this is `gcc` and `g++`, for deployment to STM32 this is `gcc-arm-none-gcc` and `gcc-arm-none-g++`. When switching compiler, CMake will frequently get confused, if this occours delete the build directory.

### Local Testing
Units tests are run with GTest. To run all tests use CTest (a part of CMake).
```sh
cd build
ctest
```

To debug a test which is failing with GDB the following procedure can be used.
```sh
# Find the command used to run the test to debug, replace [NAME_OF_TEST] with the name or a regex with matches it
ctest -R $[NAME_OF_TEST] -V -N
# Run GDB with the test command eg. gdb obc_m7  "-v" "test0"
gdb [TEST_COMMAND]
```

### Device Testing
Debugging is done with GDB.
```sh
# In a secondary terminal
st-util

# In the primary terminal
arm-none-eabi-gdb build/obc_m7.elf
target extended localhost:4242
load
```

## Multithreading
Given the low level nature of embedded development, we do not have the luxury of symmetric multiprocessing.
Each core is programmed separately and devices and memory can be mapped to specific cores. As such, within the main software project there are two sub projects (for the M4 and M7 cores).

For initial development, only a single core will be used to reduce complexity. However, code should be written in a manner such that it will be simple using both cores. This includes:
 - Using synchronization primitives where appropriate (you should be doing this anyway as FreeRTOS can preempt tasks).
 - Shifting the definition of shared primitives to the `Common/` directory, task code should be contained to a single core because it will only run there.
 - Centralizing where hardware communication occurs.

## Issues
Every change to the code should done on a separate branch which will be rebased (not merged) into `main`. Each branch should be associated with an issue and prefixed with the issue number (eg. `19-spi-bus-abstraction`).

## Linting
A limited wrapped around Clang Tidy/Format has be included into this project. All code should pass these checks (consider using a pre-commit hook).

The linter can be invoked with the target prefixed with `check-`, eg. `check-obc_m7`. This process is orders of magnitude slower than code compilation. To format code in place run:
```sh
clang-format -i -style="file:.clang-format" [FILE]
# Globs are allowed
clang-format -i -style="file:.clang-format" common/**.hpp
```
