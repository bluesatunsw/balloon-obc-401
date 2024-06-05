project(common)

set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/Src/scheduling/delay.cpp
)

if(BALLOON_CROSS_COMPILING)
    set(COMMON_SOURCES ${COMMON_SOURCES}
        ${PROJECT_SOURCE_DIR}/Src/sys/stm32/delay.cpp
        ${PROJECT_SOURCE_DIR}/Src/sys/stm32/mutex.cpp
    )
else()
    set(COMMON_SOURCES ${COMMON_SOURCES}
        ${PROJECT_SOURCE_DIR}/Src/sys/hosted/delay.cpp
    )
endif()

add_library(common INTERFACE ${COMMON_SOURCES})
target_include_directories(common INTERFACE
    ${PROJECT_SOURCE_DIR}/Inc
    # ${FREERTOS_PATH}/FreeRTOS/Source/include
)
target_link_libraries(common INTERFACE units)

if(NOT BALLOON_CROSS_COMPILING)
    add_subdirectory(Tests)
endif()

add_linter_target(common "${BALLOON_COMMON_SRCS}") # Yes, quotes necessary. CMake won't pass a list otherwise.
