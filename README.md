# balloon-obc-401
A flexible on-board-computer for high altitude ballons based on the dual core
STM32H7 series running FreeRTOS.

## Setup
The STM32CubeIDE is used for developing code for this project. It is
(effectively) required for downloading external dependencies, configuring build
commands and deploying code. However, other editors (eg. Neovim or VSC) can
still be used (reference the generated build commands to determine the include
directories to feed into your LSP).

### Library Code Generation
To install the exernal libraries for this project you need to "trick" the IDE.
1. Create a new STM32H7 dual core project with default settings.
2. Close the device configuration view, auto generate the code and build the
project.
3. Delete this dummy project.
4. Open this project and open the `.ioc` file.
5. Make any change then undo it (this will mark the configuration as dirty).
6. Close the `.ioc` file, selecting the option to save and to regenerate the
code.

## Multithreading
Given the low level nature of embedded development, we do not have the luxury
of symmetric multiprocessing; each core is programmed separately and devices
and memory can be mapped to specific cores. As such, within the main software
project there are two sub projects (for the M4 and M7 cores).

For initial development, only a single core will be used to reduce complexity.
However, code should be written in a manner such that it will be simple using
both cores. This includes:
- Using syncronisation primative where appropriate (you should be doing this
anyway as FreeRTOS can preempt tasks).
- Shifting the definition of shared primatives to the `Common/` directory, task
code should be contained to a single core because it will only run there.
- Centralising where hardware communication occours.

## Issues
Every change to the code should done on a seperate branch which will be rebased
(not merged) into `main`. Each branch should be associated with an issue and
prefixed with the issue number (eg. `19-spi-bus-abstraction`).

## Linting 
A limited wrapped around Clang Tidy/Format has be included into this project.
All code should pass these checks (consider using a pre-commit hook).

