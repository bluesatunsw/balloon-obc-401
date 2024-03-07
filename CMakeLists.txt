cmake_minimum_required(VERSION 3.28)
include(extern/stm32-cmake/cmake/stm32_gcc.cmake)
set(CMAKE_CXX_STANDARD 23)
project(balloon)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(FREERTOS_PATH "/home/evan/STM32Cube/Repository/Packs/STMicroelectronics/X-CUBE-FREERTOS/1.1.0/Middlewares/Third_Party/")
set(STM32_CUBE_H7_PATH "/home/evan/STM32Cube/Repository/STM32Cube_FW_H7_V1.11.1/")

include(extern/CMakeLists.txt)
add_subdirectory(common)

include(utils/checks.cmake)

add_subdirectory(systems)
