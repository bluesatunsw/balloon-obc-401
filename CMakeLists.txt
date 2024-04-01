cmake_minimum_required(VERSION 3.28)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER_FORCED ON)
set(CMAKE_CXX_COMPILER_FORCED ON)

project(balloon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE INTERNAL "")

if (DEFINED $ENV{STM32_RP_DIR})
    message(STATUS "[balloon-obc-401-configure] Using STM32_RP_DIR for STM32 repository directory set in environment.")
    set(STM32_RP_DIR $ENV{STM32_RP_DIR})
else()
    message(STATUS "[balloon-obc-401-configure] Using STM32_RP_DIR as default STM32CubeMX repository directory.")
    if (CMAKE_HOST_WIN32)
        string(REPLACE "\\" "/" STM32_RP_DIR "$ENV{USERPROFILE}/STM32Cube/Repository")
    elseif (CMAKE_HOST_UNIX)
        set(STM32_RP_DIR "$ENV{HOME}/STM32Cube/Repository")
    else()
        message(FATAL_ERROR "[balloon-obc-401-configure] Building on unsupported host platform! Only Win32, Linux, MacOS are supported platforms.")
    endif()
endif()

if (CMAKE_HOST_APPLE)
    message(WARNING "[balloon-obc-401-configure] MacOS is not a fully supported platform, expect errors.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extern/stm32-cmake/cmake")
set(FREERTOS_PATH "${STM32_RP_DIR}/Packs/STMicroelectronics/X-CUBE-FREERTOS/1.1.0/Middlewares/Third_Party/")
set(STM32_CUBE_H7_PATH "${STM32_RP_DIR}/STM32Cube_FW_H7_V1.11.2/")

include(extern/stm32-cmake/cmake/stm32_gcc.cmake)
include(extern/stm32-cmake/cmake/stm32/common.cmake)
include(utils/checks.cmake)

add_subdirectory(extern/units)
add_subdirectory(common)
add_subdirectory(systems)
