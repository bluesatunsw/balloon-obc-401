project(obc)

file(GLOB_RECURSE DRIVER_SOURCES ${PROJECT_SOURCE_DIR}/Drivers/*.c)
file(GLOB_RECURSE MIDDLEWARE_SOURCES ${PROJECT_SOURCE_DIR}/Middlewares/*.c)
set(CM7_SOURCES
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/cppmain.cpp
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/freertos.c
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/main.c
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/stm32h7xx_hal_msp.c
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/stm32h7xx_it.c
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/syscalls.c
    ${PROJECT_SOURCE_DIR}/CM7/Core/Src/sysmem.c
    ${DRIVER_SOURCES}
    ${MIDDLEWARE_SOURCES}
)

set(CM7_INCLUDES
    ${PROJECT_SOURCE_DIR}/CM7/Core/Inc
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/include
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    ${PROJECT_SOURCE_DIR}/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    ${PROJECT_SOURCE_DIR}/Drivers/CMSIS/Include
    ${PROJECT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32H7xx/Include
    ${PROJECT_SOURCE_DIR}/Drivers/STM32H7xx_HAL_Driver/Inc
)

system_kernel(obc_cm7
    "${CM7_SOURCES}"
    "${CM7_INCLUDES}"
    "${PROJECT_SOURCE_DIR}/CM7/STM32H745ZITX_FLASH.ld"
    "-mcpu=cortex-m7;-mfloat-abi=hard;-mfpu=fpv4-sp-d16"
    "-DSTM32H745xx;-DCORE_CM7"
)
