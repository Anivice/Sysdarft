message(STATUS "CMake Module Python Virtual Environment Loaded")

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")

set(VENV_DIR "${CMAKE_BINARY_DIR}/PythonVirtualEnvironment")

check_program("bash" "BASH")
check_program("python3" "PYTHON3")

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
