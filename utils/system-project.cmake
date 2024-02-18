macro(system_kernel kern_name sources includes link_script cpu_flags compile_flags)
    message(${CMAKE_CXX_COMPILER})
    if ($ENV{BUILD_PASS} STREQUAL "arm")
        add_executable(${kern_name} ${COMMON_SOURCES} ${sources})
        target_include_directories(${kern_name} PRIVATE
            "${COMMON_INCLUDES};${includes}"
        )
        target_compile_options(
            ${kern_name} PUBLIC
            ${COMMON_COMPILE_FLAGS} ${cpu_flags} ${compile_flags}
            -mthumb -ffunction-sections -fdata-sections -fno-common
        )
        target_link_options(${kern_name} PUBLIC
            -T${link_script} ${cpu_flags}
            -Wl,-Map=${kern_name}.map
            --specs=nosys.specs
            -Wl,--gc-sections
            -u _printf_float
            -Wl,--start-group
            -lc -lm
            -lstdc++ -lsupc++
            -Wl,--end-group
            -Wl,--print-memory-usage
        )

        add_custom_command(TARGET ${kern_name} POST_BUILD
            COMMAND ${CMAKE_OBJCOPY} -O ihex ${kern_name}.elf ${kern_name}.hex
            BYPRODUCTS ${kern_name}.hex
            COMMENT "Building ${kern_name}.hex"
        )
    endif()
endmacro()
