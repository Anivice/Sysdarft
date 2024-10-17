message(STATUS "CMake Module Python Virtual Environment Loaded")

set(VENV_DIR "${CMAKE_BINARY_DIR}/PythonVirtualEnvironment")

# Ensure Python 3 is installed
find_program(PYTHON3_EXECUTABLE python3)
if(NOT PYTHON3_EXECUTABLE)
    message(FATAL_ERROR "Python 3 was not found! Please install Python 3 to proceed.")
else()
    message(STATUS "Python 3 found: ${PYTHON3_EXECUTABLE}")
    # Optionally check if Python is usable by running a simple version command
    execute_process(
            COMMAND ${PYTHON3_EXECUTABLE} --version
            OUTPUT_VARIABLE python_version_output
            ERROR_VARIABLE python_version_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    if(NOT python_version_output)
        message(FATAL_ERROR "Python 3 is installed but not usable.")
    else()
        message(STATUS "Python 3 version: ${python_version_output}")
    endif()
endif()

# Ensure Bash is installed
find_program(BASH_EXECUTABLE bash)
if(NOT BASH_EXECUTABLE)
    message(FATAL_ERROR "Bash was not found! Please install Bash to proceed.")
else()
    message(STATUS "Bash found: ${BASH_EXECUTABLE}")
    # Optionally check if Bash is usable by running a simple version command
    execute_process(
            COMMAND ${BASH_EXECUTABLE} --version
            OUTPUT_VARIABLE bash_version_output
            ERROR_VARIABLE bash_version_error
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    if(NOT bash_version_output)
        message(FATAL_ERROR "Bash is installed but not usable.")
    else()
        message(STATUS "Bash version: ${bash_version_output}")
    endif()
endif()

message(STATUS "Creating Python virtual environment...")
execute_process(
        COMMAND ${PYTHON3_EXECUTABLE} -m venv "${VENV_DIR}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Activating virtual environment and installing packages...")
execute_process(
        COMMAND ${BASH_EXECUTABLE} -c "source ${VENV_DIR}/bin/activate; pip install --upgrade pip && pip install -r ${CMAKE_SOURCE_DIR}/requirements.txt"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "Setup virtual environment at ${VENV_DIR} completed.")
set(PythonVirtualEnvironment_DIR "${VENV_DIR}")
