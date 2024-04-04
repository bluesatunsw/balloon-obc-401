cmake_minimum_required(VERSION 3.28)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER_FORCED ON)
set(CMAKE_CXX_COMPILER_FORCED ON)

project(balloon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE INTERNAL "")

if (CMAKE_HOST_APPLE)
    message(WARNING "[balloon-obc-401-configure] MacOS is not a fully supported platform, expect errors.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/extern/stm32-cmake/cmake")
set(STM32_CUBE_H7_PATH "${CMAKE_SOURCE_DIR}/extern/STM32CubeH7")    
set(FREERTOS_PATH "${CMAKE_SOURCE_DIR}/extern/x-cube-freertos/Middlewares/Third_Party")

include(extern/stm32-cmake/cmake/stm32_gcc.cmake)
include(extern/stm32-cmake/cmake/stm32/common.cmake)
include(utils/checks.cmake)

add_subdirectory(extern/units)
add_subdirectory(common)
add_subdirectory(systems)
