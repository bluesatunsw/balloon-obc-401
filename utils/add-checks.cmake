function(add_linter_target target_name sources)
    set(TARGET_NAME ${target_name})

    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} not found, can't be checked.")
    endif()

    get_target_property(TARGET_INCLUDE_DIRS ${TARGET_NAME} INCLUDE_DIRECTORIES)
    get_target_property(TARGET_INTERFACE_INCLUDE_DIRS ${TARGET_NAME} INTERFACE_INCLUDE_DIRECTORIES)
    if (TARGET_INCLUDE_DIRS)
        list(APPEND TARGET_ALL_INCLUDE_DIRS ${TARGET_INCLUDE_DIRS})
    endif()
    if (TARGET_INTERFACE_INCLUDE_DIRS)
        list(APPEND TARGET_ALL_INCLUDE_DIRS ${TARGET_INTERFACE_INCLUDE_DIRS})
    endif()
    list(REMOVE_DUPLICATES TARGET_ALL_INCLUDE_DIRS)

    get_target_property(TARGET_COMPILE_DEFS ${TARGET_NAME} COMPILE_DEFINITIONS)
    if (NOT TARGET_COMPILE_DEFS)
        set(TARGET_COMPILE_DEFS "")
    endif()

    add_custom_target(check-${TARGET_NAME}
        ALL
        # Yes, quote whole argument, otherwise list semicolons are omitted by CMake.
        ${CMAKE_COMMAND} "-DCHECK_SRCS=${sources}" "-DCHECK_INCLUDE_DIRS=${TARGET_ALL_INCLUDE_DIRS}" "-DCHECK_DEFINITIONS=${TARGET_COMPILE_DEFS}" -P "${CMAKE_SOURCE_DIR}/utils/run-checks.cmake"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VERBATIM
    )
endfunction()