project(common)

set(COMMON_SOURCES
    ${PROJECT_SOURCE_DIR}/Src/scheduling/delay.cpp
)
set(COMMON_HEADERS
    ${PROJECT_SOURCE_DIR}/Inc/obc/ipc/callback.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/ipc/mutex.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/scheduling/delay.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/scheduling/task.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/utils/error.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/utils/handle.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/utils/meta.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/bus.hpp
    ${PROJECT_SOURCE_DIR}/Inc/obc/port.hpp
)

if(BALLOON_CROSS_COMPILING)
    list(APPEND COMMON_SOURCES
        ${PROJECT_SOURCE_DIR}/Src/sys/stm32/delay.cpp
        ${PROJECT_SOURCE_DIR}/Src/sys/stm32/mutex.cpp
    )
    list(APPEND COMMON_HEADERS
        ${PROJECT_SOURCE_DIR}/Inc/obc/sys/stm32/delay.hpp
        ${PROJECT_SOURCE_DIR}/Inc/obc/sys/stm32/mutex.hpp
    )
else()
    list(APPEND COMMON_SOURCES
        ${PROJECT_SOURCE_DIR}/Src/sys/hosted/delay.cpp
    )
    list(APPEND COMMON_HEADERS
        ${PROJECT_SOURCE_DIR}/Inc/obc/sys/hosted/delay.hpp
        ${PROJECT_SOURCE_DIR}/Inc/obc/sys/hosted/mutex.hpp
    )
endif()

add_library(common INTERFACE ${COMMON_SOURCES})
target_include_directories(common INTERFACE ${PROJECT_SOURCE_DIR}/Inc)
target_link_libraries(common INTERFACE units)

if(NOT BALLOON_CROSS_COMPILING)
    set(TO_LINT ${COMMON_SOURCES})
    list(APPEND TO_LINT ${COMMON_HEADERS})
    # Currently the STM32 dependencies are not actually present
    add_linter_target(common  "${TO_LINT}")
    add_subdirectory(Tests)
endif()
