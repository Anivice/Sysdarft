message(STATUS "CMake Module check_program Loaded")

# Helper function to check if a program is installed and usable
function(check_program prog_name prog_exec)
    find_program(${prog_exec}_EXECUTABLE ${prog_name} REQUIRED)
    if (NOT ${prog_exec}_EXECUTABLE)
        message(FATAL_ERROR "${prog_name} was not found! Please install ${prog_name} to proceed.")
    else()
        message(STATUS "${prog_name} found: ${${prog_exec}_EXECUTABLE}")
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
            message(STATUS "${prog_name} version: ${${prog_exec}_version_output}")
        endif()
    endif()
endfunction()
