# balloon-obc-401
A flexible on-board-computer for high altitude balloons based on the dual core STM32H7 series running FreeRTOS.

## Setup / Build Instructions
1. Windows -> ARM cross toolchain available [here](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).
2. Run `git submodule update --init --remote --recursive`
3. `cmake -DCMAKE_C_COMPILER=... -DCMAKE_CXX_COMPILER=... -Bbuild .`
4. `cmake --build build --config Debug --target all`
5. Profit

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
A limited wrapped around Clang Tidy/Format has be included into this project.
All code should pass these checks (consider using a pre-commit hook).
