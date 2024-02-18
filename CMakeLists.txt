cmake_minimum_required(VERSION 3.28)
include(utils/compiler-setup.cmake)
project(balloon)

include(utils/compiler-setup.cmake)
include(utils/system-project.cmake)
include(utils/checks.cmake)

add_subdirectory(extern)
add_subdirectory(common)
add_subdirectory(systems)
