set(MODULE_NAME "PythonVirtualEnv")
set(LOADED_MODULE_LIST "${LOADED_MODULE_LIST}")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    if ("X${Module}" STREQUAL "X${MODULE_NAME}")
        return()
    endif ()
endforeach (Module IN LISTS LOADED_MODULE_LIST)
list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")

message(STATUS "CMake Module Python Virtual Environment Loaded")

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/color_the_console.cmake")

set(VENV_DIR "${CMAKE_BINARY_DIR}/PythonVirtualEnvironment")

check_program("bash" "BASH")
check_program("python3" "PYTHON3")

if ("${DISABLE_PYTHON3_VENV_SETUP}" STREQUAL "False")
    if(NOT EXISTS "${VENV_DIR}")
        console_turn_blue()
        message(STATUS "Creating Python virtual environment...")
        console_reset_color()
        execute_process(
                COMMAND ${PYTHON3_EXECUTABLE} -m venv "${VENV_DIR}"
                WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
    else ()
        console_turn_blue()
        message(STATUS "Skipping creation of virtual environment since it exists already.")
        message(STATUS "If you want to recreate the virtual environment, run `rm -rf \"${VENV_DIR}\"`\n\tbefore initialize CMake again.")
        console_reset_color()
    endif ()
endif ()

console_turn_blue()
message(STATUS "Activating virtual environment and installing packages...")
console_reset_color()

console_turn_dim()
if (${SUPRESS_DEBUG_INFO} STREQUAL "True")
    execute_process(
            COMMAND ${BASH_EXECUTABLE} -c "         \
            source ${VENV_DIR}/bin/activate;        \
            pip install --upgrade pip > /dev/null;  \
            pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt > /dev/null"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
else ()
    execute_process(
            COMMAND ${BASH_EXECUTABLE} -c " \
            source ${VENV_DIR}/bin/activate; \
            pip install --upgrade pip && pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif ()


console_reset_color()

console_turn_green()
message(STATUS "Setup virtual environment at ${VENV_DIR} completed.")
console_reset_color()
set(PythonVirtualEnvironment_DIR "${VENV_DIR}")

message(STATUS "CMake Module Python Virtual Environment loading completed")
