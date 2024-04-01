cmake_minimum_required(VERSION 3.28)

set(CMAKE_C_COMPILER_FORCED ON)
set(CMAKE_CXX_COMPILER_FORCED ON)

project(balloon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE INTERNAL "")

if (CMAKE_HOST_WIN32)
    string(REPLACE "\\" "/" USER_HOME_DIR "$ENV{USERPROFILE}")
elseif (CMAKE_HOST_UNIX AND NOT CMAKE_HOST_APPLE)
    set(USER_HOME_DIR "$ENV{HOME}")
else()
    message(FATAL_ERROR "[balloon-obc-401-configure] Only Windows and Linux are supported platforms.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extern/stm32-cmake/cmake")
set(FREERTOS_PATH "${USER_HOME_DIR}/STM32Cube/Repository/Packs/STMicroelectronics/X-CUBE-FREERTOS/1.1.0/Middlewares/Third_Party/")
set(STM32_CUBE_H7_PATH "${USER_HOME_DIR}/STM32Cube/Repository/STM32Cube_FW_H7_V1.11.2/")

include(extern/stm32-cmake/cmake/stm32_gcc.cmake)
include(extern/stm32-cmake/cmake/stm32/common.cmake)
include(utils/checks.cmake)

add_subdirectory(extern/units)
add_subdirectory(common)
add_subdirectory(systems)
