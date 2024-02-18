project(obc)

set(CM7_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/CM7/Core/Src/cppmain.cpp
)
set(CM7_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/CM7/Core/Inc
)
system_kernel(obc_cm7
    "${CM7_SOURCES}"
    "${CM7_INCLUDES}"
    "CM7/STM32H75ZITX_FLASH.ld"
    "-mcpu=cortex-m7;-mfloat-abi=hard;-mfpu=fpv4-sp-d16"
)
