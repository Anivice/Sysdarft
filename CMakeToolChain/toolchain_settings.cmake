# toolchain_settings.cmake

# Find ccache
find_program(CCACHE_PROGRAM ccache)
if(NOT CCACHE_PROGRAM)
    message(FATAL_ERROR "ccache not found! Please install ccache or adjust the configuration.")
endif()

# Set ccache as the compiler launcher
set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C Compiler Launcher" FORCE)
set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C++ Compiler Launcher" FORCE)
