set(MODULE_NAME "ConsoleColor")
set(LOADED_MODULE_LIST "${LOADED_MODULE_LIST}")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    if ("X${Module}" STREQUAL "X${MODULE_NAME}")
        return()
    endif ()
endforeach (Module IN LISTS LOADED_MODULE_LIST)
list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")

message(STATUS "CMake Module Console Color Loaded")

find_program(BASH_EXECUTABLE bash REQUIRED)

function(console_turn_red)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[31m\"")
endfunction()

function(console_turn_dim)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[2m\"")
endfunction()

function(console_turn_blue)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[34m\"")
endfunction()

function(console_turn_green)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[32m\"")
endfunction()

function(console_turn_yellow)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[33m\"")
endfunction()

function(console_reset_color)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[0m\"")
endfunction()

message(STATUS "CMake Module Console Color loading completed")
