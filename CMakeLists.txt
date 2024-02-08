project(balloon)

include(utils/compiler-setup.cmake)
include(utils/lib-loader.cmake)
include(utils/linter-task.cmake)
include(utils/git-hooks.cmake)

add_subdirectory(common)
add_subdirectory(systems)

