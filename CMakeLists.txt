cmake_minimum_required(VERSION 3.10)

project(Sysdarft C CXX)

enable_testing()

include(cmake_modules/utests.cmake)

include_directories(include)

add_library(sysdarft_debug STATIC
        include/debug.h
        src/debug/stack_frame_tracer.cpp
        src/debug/sysdarft_error.cpp
)

link_libraries(sysdarft_debug)

sysdarft_add_test(cmake_sysdarft_test
        "utests/cmake_sysdarft_test_case_test.cpp"
    )
