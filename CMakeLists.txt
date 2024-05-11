cmake_minimum_required(VERSION 3.27)

if (CMAKE_C_COMPILER MATCHES "arm-none-eabi" OR CMAKE_CXX_COMPILER MATCHES "arm-none-eabi")
    set(CMAKE_SYSTEM_NAME "Generic")
    set(CMAKE_C_COMPILER_FORCED ON)
    set(CMAKE_CXX_COMPILER_FORCED ON)

    set(BALLOON_CROSS_COMPILING ON)
    add_compile_definitions(BALLOON_STM32)
else()
    set(BALLOON_CROSS_COMPILING OFF)
    add_compile_definitions(BALLOON_HOSTED)
endif()

project(balloon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE INTERNAL "")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

if (CMAKE_HOST_APPLE)
    message(WARNING "[balloon-obc-401-configure] MacOS is not a fully supported platform, expect errors.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extern/stm32-cmake/cmake")
set(STM32_CUBE_H7_PATH "${CMAKE_SOURCE_DIR}/extern/STM32CubeH7")    
set(FREERTOS_PATH "${CMAKE_SOURCE_DIR}/extern/x-cube-freertos/Middlewares/Third_Party")

if (BALLOON_CROSS_COMPILING)
    include(extern/stm32-cmake/cmake/stm32_gcc.cmake)
    include(extern/stm32-cmake/cmake/stm32/common.cmake)
else()
    set(INSTALL_GTEST OFF CACHE INTERNAL "" FORCE)
    add_subdirectory(extern/googletest)

    enable_testing()
endif()

include(utils/checks.cmake)

add_subdirectory(extern/units)

add_subdirectory(common)
add_subdirectory(systems)
