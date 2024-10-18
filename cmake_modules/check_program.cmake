set(MODULE_NAME "CheckProgram")
set(LOADED_MODULE_LIST "${LOADED_MODULE_LIST}")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    if ("X${Module}" STREQUAL "X${MODULE_NAME}")
        return()
    endif ()
endforeach (Module IN LISTS LOADED_MODULE_LIST)
list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")

message(STATUS "CMake Module check_program Loaded")

include("${CMAKE_CURRENT_LIST_DIR}/color_the_console.cmake")

# Helper function to check if a program is installed and usable
function(check_program prog_name prog_exec)
    find_program(${prog_exec}_EXECUTABLE ${prog_name} REQUIRED)
    if (NOT ${prog_exec}_EXECUTABLE)
        message(FATAL_ERROR "${prog_name} was not found! Please install ${prog_name} to proceed.")
    else()
        console_turn_green()
        message(STATUS "${prog_name} found: ${${prog_exec}_EXECUTABLE}")
        console_reset_color()

        execute_process(
                COMMAND ${${prog_exec}_EXECUTABLE} --version
                OUTPUT_VARIABLE ${prog_exec}_version_output
                ERROR_VARIABLE ${prog_exec}_version_error
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_STRIP_TRAILING_WHITESPACE
                RESULT_VARIABLE ${prog_exec}_exit_status
        )
        if (NOT ${prog_exec}_exit_status EQUAL 0)
            message(FATAL_ERROR "${prog_name} is installed but not usable. Error: ${${prog_exec}_version_error}")
        else()
            if (${SUPRESS_DEBUG_INFO} STREQUAL "True")
                set(CMAKE_MESSAGE_LOG_LEVEL WARNING)
            endif ()

            console_turn_dim()
            if (${prog_exec}_version_output)
                message(NOTICE "${prog_name} version: ${${prog_exec}_version_output}")
            else ()
                message(NOTICE "${prog_name} version: ${${prog_exec}_version_error}")
            endif ()
            console_reset_color()

            set(CMAKE_MESSAGE_LOG_LEVEL NOTICE)
        endif()
    endif()
endfunction()

message(STATUS "CMake Module check_program loading competed")
