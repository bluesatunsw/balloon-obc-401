# CLI Args
# CHECK_SRCS: List of source files to check.
# CHECK_INCLUDE_DIRS: List of include directories to use for sources passed to clang-tidy.
# CHECK_DEFINITIONS: List of compile definitions to use for sources passed to clang-tidy.

if (NOT DEFINED CHECK_SRCS)
    message(FATAL_ERROR "Missing required CHECK_SRCS argument.")
endif()
if (NOT DEFINED CHECK_INCLUDE_DIRS)
    message(FATAL_ERROR "Missing required CHECK_INCLUDE_DIRS argument.")
endif()
if (NOT DEFINED CHECK_DEFINITIONS)
    message(FATAL_ERROR "Missing required CHECK_DEFINITIONS argument.")
endif()
if (NOT DEFINED USE_FULL_CLANG_TIDY_RUN)
    set(USE_FULL_CLANG_TIDY_RUN OFF)
endif()

# Check for license headers.

file(READ utils/license-header. BALLOON_LICENSE_HEADER)

foreach(SOURCE_FILE ${CHECK_SRCS})
    file(READ "${SOURCE_FILE}" SOURCE_CONTENTS)
    string(FIND "${SOURCE_CONTENTS}" "${BALLOON_LICENSE_HEADER}" SEARCH_POS)
    if (NOT SEARCH_POS EQUAL 0)
        message(FATAL_ERROR "License header not found in ${SOURCE_FILE}. Yes, I'm making you add this if you want CMake to build.")
    endif()
endforeach()

# clang-format

find_program(CLANG_FORMAT_COMMAND "clang-format")
if (CLANG_FORMAT_COMMAND)
    foreach(SOURCE_FILE ${CHECK_SRCS})
        execute_process(
            COMMAND "${CLANG_FORMAT_COMMAND}" --Werror "--style=file:${CMAKE_CURRENT_LIST_DIR}/../.clang-format" --fallback-style=Google -i "${SOURCE_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}"
        )
    endforeach()
else()
    message(WARNING "clang-format not found. Skipping formatting checks.")
endif()

# clang-tidy

set(INCLUDES_CLI_ARGS "")
foreach(INCLUDE ${CHECK_INCLUDE_DIRS})
    set(INCLUDES_CLI_ARGS ${INCLUDES_CLI_ARGS} -I ${INCLUDE})
endforeach()
set(DEFS_CLI_ARGS "")
foreach(DEF ${CHECK_DEFINITIONS})
    set(DEFS_CLI_ARGS ${DEFS_CLI_ARGS} -D ${DEF})
endforeach()

message(STATUS ${INCLUDES_CLI_ARGS})
find_program(CLANG_TIDY_COMMAND "clang-tidy")
if (CLANG_TIDY_COMMAND)
    foreach(SOURCE_FILE ${CHECK_SRCS})
        execute_process(
            COMMAND "${CLANG_TIDY_COMMAND}" "${SOURCE_FILE}" -header-filter=.* -- -std=c++23 ${INCLUDES_CLI_ARGS} ${DEFS_CLI_ARGS}
            COMMAND_ECHO STDOUT
        )
    endforeach()
else()
    message(WARNING "clang-tidy not found. Skipping formatting checks.")
endif()
