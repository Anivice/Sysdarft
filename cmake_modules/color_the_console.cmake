set(MODULE_NAME "ConsoleColor")
if (NOT "${MODULE_NAME}" IN_LIST LOADED_MODULE_LIST)
    list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")
    message(STATUS "CMake Module Console Color Loaded")
else()
    return()
endif()

find_program(BASH_EXECUTABLE bash REQUIRED)

# Generic function to change console color
function(console_set_color color_code)
    execute_process(COMMAND ${BASH_EXECUTABLE} -c "echo -ne \"\\033[${color_code}m\"")
endfunction()

# Color functions using the generic color setter
function(console_turn_red)
    console_set_color("31")
endfunction()

function(console_turn_dim)
    console_set_color("2;3")
endfunction()

function(console_turn_blue)
    console_set_color("34")
endfunction()

function(console_turn_green)
    console_set_color("32")
endfunction()

function(console_turn_yellow)
    console_set_color("33")
endfunction()

function(console_reset_color)
    console_set_color("0")
endfunction()

message(STATUS "CMake Module Console Color loading completed")
