project(obc CXX C ASM)

find_package(CMSIS 2 COMPONENTS
    STM32H755ZI_M4 STM32H755ZI_M7
    REQUIRED
)
find_package(HAL COMPONENTS
    STM32H755ZI_M4 STM32H755ZI_M7
    REQUIRED
)
find_package(FreeRTOS COMPONENTS ARM_CM7 ARM_CM4F REQUIRED)

set(OBC_COMMON_SOURCES
    ${COMMON_SOURCES}
)

set(M7_SOURCES
    CM7/Core/Src/freertos.c
    CM7/Core/Src/main.c
    CM7/Core/Src/stm32h7xx_hal_msp.c
    CM7/Core/Src/stm32h7xx_it.c
)

set(M4_SOURCES
    CM4/Core/Src/freertos.c
    CM4/Core/Src/main.c
    CM4/Core/Src/stm32h7xx_hal_msp.c
    CM4/Core/Src/stm32h7xx_it.c
)

add_executable(obc_m7 ${OBC_COMMON_SOURCES} ${M7_SOURCES})
add_executable(obc_m4 ${OBC_COMMON_SOURCES} ${M4_SOURCES})
target_include_directories(obc_m7 PRIVATE ${COMMON_INCLUDES} CM7/Core/Inc/)
target_include_directories(obc_m4 PRIVATE ${COMMON_INCLUDES} CM4/Core/Inc/)

target_link_libraries(obc_m7 PRIVATE
    HAL::STM32::H7::M7::RCCEx
    HAL::STM32::H7::M7::GPIO
    HAL::STM32::H7::M7::DMA
    HAL::STM32::H7::M7::MDMA
    HAL::STM32::H7::M7::ETH
    HAL::STM32::H7::M7::EXTI
    HAL::STM32::H7::M7::CORTEX
    HAL::STM32::H7::M7::FLASH
    HAL::STM32::H7::M7::HSEM
    HAL::STM32::H7::M7::I2C
    HAL::STM32::H7::M7::PWREx
    HAL::STM32::H7::M7::UARTEx
    HAL::STM32::H7::M7::PCDEx
    HAL::STM32::H7::M7::LL_USB
    FreeRTOS::ARM_CM7
    FreeRTOS::Timers
    FreeRTOS::Heap::4
    CMSIS::STM32::H755ZI::M7
    CMSIS::STM32::H7::M7::RTOS_V2
    STM32::NoSys
)
target_link_directories(obc_m7 INTERFACE
    FreeRTOS::ARM_CM7
    CMSIS::STM32::H755ZI::M7
)
target_link_libraries(obc_m4 PRIVATE
    HAL::STM32::H7::M4::RCC
    HAL::STM32::H7::M4::GPIO
    HAL::STM32::H7::M4::DMA
    HAL::STM32::H7::M4::MDMA
    HAL::STM32::H7::M4::EXTI
    HAL::STM32::H7::M4::CORTEX
    HAL::STM32::H7::M4::FLASH
    HAL::STM32::H7::M4::HSEM
    HAL::STM32::H7::M4::PWREx
    HAL::STM32::H7::M4::UART
    FreeRTOS::ARM_CM4F
    FreeRTOS::Timers
    FreeRTOS::Heap::4
    CMSIS::STM32::H755ZI::M4
    CMSIS::STM32::H7::M4::RTOS_V2
    STM32::NoSys
)
target_link_directories(obc_m4 INTERFACE
    FreeRTOS::ARM_CM4F
    CMSIS::STM32::H755ZI::M4
)
