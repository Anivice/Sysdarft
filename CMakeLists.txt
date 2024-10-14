cmake_minimum_required(VERSION 3.18)

project(Sysdarft C CXX)

# Setup language standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

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

# Universal include directory
include_directories(include)

# ##1: Sysdarft debug library
add_library(sysdarft_debug STATIC
        include/debug.h
        src/debug/stack_frame_tracer.cpp
        src/debug/sysdarft_error.cpp
)
if (WIN32)
    add_compile_definitions(SYSDARFT_BUILD_TARGET_PLATFORM_IS_WINDOWS)
elseif (UNIX)
    target_compile_options(sysdarft_debug PUBLIC -Wno-deprecated -frtti)
endif()
link_libraries(sysdarft_debug)

# Unit tests
sysdarft_add_test(cmake_sysdarft_test
        utests/cmake_sysdarft_test_case_test.cpp
)
