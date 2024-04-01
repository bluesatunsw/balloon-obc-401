project(common)

add_library(common INTERFACE
    ${PROJECT_SOURCE_DIR}/Src/ipc/mutex.cpp
    ${PROJECT_SOURCE_DIR}/Src/scheduling/delay.cpp
)
target_include_directories(common INTERFACE
    ${PROJECT_SOURCE_DIR}/Inc/obc
)
