cmake_minimum_required(VERSION 3.18)

# Setup language standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

project(Sysdarft C CXX)

# Check if the compiler is GCC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "GCC is required as the compiler.")
endif()

enable_testing()

# Platform check
if (NOT UNIX)
    message(FATAL_ERROR "Unknown target platform!")
endif()

# Build type check
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(SYSDARFT_TARGET_IS_DEBUG_BUILD)
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SYSDARFT_TARGET_IS_RELEASE_BUILD)
endif()

# Include CMake submodules
include(cmake_modules/utests.cmake)
find_package(Python3 COMPONENTS Interpreter Development)
set(PythonVirtualEnvironment_DIR)
include(cmake_modules/python_venv.cmake)
message(STATUS "Python virtual environment is setup at `${PythonVirtualEnvironment_DIR}`")

# Universal include directory
include_directories(include)

add_compile_definitions("CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_compile_definitions("CMAKE_BINARY_DIR=\"${CMAKE_BINARY_DIR}\"")

# ##1: Sysdarft debug library
add_library(sysdarft_debug STATIC
        include/debug.h
        src/debug/stack_frame_tracer.cpp
        src/debug/sysdarft_error.cpp
)
target_compile_options(sysdarft_debug PUBLIC -Wno-deprecated -frtti -g3 -gdwarf-4 -O0)
link_libraries(sysdarft_debug)

# ##2: Amber Screen Display
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" TRIMMED_Python3_VERSION "${Python3_VERSION}")
list(APPEND CMAKE_PREFIX_PATH "${PythonVirtualEnvironment_DIR}/lib/python${TRIMMED_Python3_VERSION}/site-packages/pybind11/share/cmake/pybind11")
message(STATUS "pybind11 set to ${PythonVirtualEnvironment_DIR}/lib/python${TRIMMED_Python3_VERSION}/site-packages/pybind11/share/cmake/pybind11")
find_package(pybind11 REQUIRED)
set(PYBIND11_CPP_STANDARD -std=c++20)
link_libraries(pybind11::embed Python3::Python)
add_library(sysdarft_amber_phosphor_screen STATIC
    include/sysdarft_display.h
    src/display/sysdarft_display.cpp
)
link_libraries(sysdarft_amber_phosphor_screen)

# Unit tests
sysdarft_add_test(cmake_sysdarft_test
        utests/cmake_sysdarft_test_case_test.cpp
)

sysdarft_add_test(amber_phosphor_screen_test
    utests/amber_phosphor_screen_test.cpp
)

sysdarft_add_test(sysdarft_display
    utests/sysdarft_display_test.cpp
)
