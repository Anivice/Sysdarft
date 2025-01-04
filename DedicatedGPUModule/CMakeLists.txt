cmake_minimum_required(VERSION 3.14)
project(VNCTexServer VERSION 1.0 LANGUAGES CXX)

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find libvncserver using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBVNCSERVER REQUIRED libvncserver)

if(NOT LIBVNCSERVER_FOUND)
    message(FATAL_ERROR "libvncserver not found. Please install it.")
endif()

# Include directories
include_directories(${LIBVNCSERVER_INCLUDE_DIRS})

# Add executable
add_executable(vnc_text_server src/main.cpp)

# Link libraries
target_link_libraries(vnc_text_server ${LIBVNCSERVER_LIBRARIES})

# Compiler options
target_compile_options(vnc_text_server PRIVATE ${LIBVNCSERVER_CFLAGS_OTHER})
