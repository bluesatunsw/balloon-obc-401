set(COMMON_EXTERN_SOURCES
    ""
)
set(COMMON_EXTERN_INCLUDES
    ${PROJECT_SOURCE_DIR}/extern/units/include
)

list(APPEND CMAKE_MODULE_PATH
    extern/stm32-cmake/cmake
)

include(extern/stm32-cmake/cmake/stm32/common.cmake)
