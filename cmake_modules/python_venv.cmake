set(MODULE_NAME "PythonVirtualEnv")
if (NOT "${MODULE_NAME}" IN_LIST LOADED_MODULE_LIST)
    list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")
    message(STATUS "CMake Module Python Virtual Environment Loaded")
else()
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/color_the_console.cmake")

set(VENV_DIR "${CMAKE_BINARY_DIR}/PythonVirtualEnvironment")

# Check required programs
check_program("bash" "BASH")
check_program("python3" "PYTHON3")

# Create virtual environment if needed
if ("${DISABLE_PYTHON3_VENV_SETUP}" STREQUAL "False")
    if (NOT EXISTS "${VENV_DIR}")
        console_turn_blue()
        message(STATUS "Creating Python virtual environment...")
        console_reset_color()
        execute_process(COMMAND ${PYTHON3_EXECUTABLE} -m venv "${VENV_DIR}")
    else()
        console_turn_blue()
        message(STATUS "Virtual environment already exists. To recreate, run `rm -rf \"${VENV_DIR}\"` and reinitialize CMake.")
        console_reset_color()
    endif()

    # Activate virtual environment and install packages
    console_turn_blue()
    message(STATUS "Activating virtual environment and installing packages...")
    console_reset_color()

    console_turn_dim()
    if (${SUPRESS_DEBUG_INFO} STREQUAL "True")
        execute_process(
                COMMAND ${BASH_EXECUTABLE} -c "source ${VENV_DIR}/bin/activate && pip install --upgrade pip > /dev/null && pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt > /dev/null"
        )
    else()
        execute_process(
                COMMAND ${BASH_EXECUTABLE} -c "source ${VENV_DIR}/bin/activate && pip install --upgrade pip && pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt"
        )
    endif()
    console_reset_color()

    console_turn_green()
    message(STATUS "Setup virtual environment at ${VENV_DIR} completed.")
    console_reset_color()

    set(PythonVirtualEnvironment_DIR "${VENV_DIR}")
endif()

message(STATUS "CMake Module Python Virtual Environment loading completed")
