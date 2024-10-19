cmake_minimum_required(VERSION 3.18)

# Setup language standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD 99)

project(Sysdarft C CXX)

# Check if the compiler is GCC
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "GCC is required as the compiler.")
endif()

set(compiler_options
        # Warnings and diagnostics
        -Wall                                # Enable common warnings
        -Wextra                              # Enable extra warnings
        -Wpedantic                           # Strict compliance with the standard
        -Wunused                             # Warn about unused variables, functions, etc.
        -Wuninitialized                      # Warn if variables are used uninitialized
        -fdiagnostics-show-option            # Show which option triggered the warning
        -fdiagnostics-color=always           # Enable colored diagnostics for better readability

        # Debugging and stack protection
        -g3                                  # Maximum debug information, including macro expansions
        -O0                                  # No optimization, easier for debugging
        -fstack-usage                        # Generate stack usage info for each function
        -fstack-protector-all                # Protect all functions with a stack canary to prevent stack overflow attacks
        -D_FORTIFY_SOURCE=2                  # Buffer overflow detection on safer libc functions (e.g., memcpy)
        -gdwarf-4

        # Sanitize memory and thread issues
        -fsanitize=address                   # Detect illegal memory access such as buffer overflows and use-after-free
        -fsanitize=undefined                 # Detect undefined behavior like integer overflows and null dereferencing
#        -fsanitize=thread                    # Ensure thread safety by detecting data races

        # Linker options
#        -Wl,--verbose                        # Verbose linker output to show linking process details
        -flto                                # Enable Link-Time Optimization, but keep it compatible with debug builds
)

set(linker_options
        # Linker options for memory safety, thread safety, and verbose debugging
        -fuse-ld=gold                           # Use the gold linker for faster linking (optional, depending on system availability)
        -Wl,--no-omagic                         # Prevent the generation of object files in memory; useful for debugging
        -Wl,--as-needed                         # Only link libraries that are actually needed to reduce binary size
        -Wl,--fatal-warnings                    # Treat all linker warnings as errors to catch issues early
        -Wl,-z,relro                            # Read-only relocations to prevent certain memory exploits (optional)
        -Wl,-z,now                              # Fully resolve all symbols during the link time for extra safety
        -Wl,-z,noexecstack                      # Prevent execution of code on the stack (security hardening)
        -Wl,-z,defs                             # Ensure all symbols are defined, and prevent undefined symbols
        -Wl,-O1                                 # Optimize the linking process while maintaining debuggability

        -gdwarf-4

        # AddressSanitizer and ThreadSanitizer linking (to ensure proper linking of sanitizers)
        -fsanitize=address                      # Link the AddressSanitizer runtime for memory integrity
#        -fsanitize=thread                       # Link the ThreadSanitizer runtime for thread safety
        -fsanitize=undefined                    # Link the UndefinedBehaviorSanitizer for detecting undefined behavior

        # Stack protection and LTO options
        -fstack-protector-all                   # Link with stack protection for all functions
        -flto                                   # Enable link-time optimization (can be disabled during debug for clarity)
)

add_compile_options(${compiler_options})
add_link_options(${linker_options})

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

function(auto_set_flag flag_name default_var)
    set(${flag_name} "${${flag_name}}")
    if (NOT ${flag_name})
        set(${flag_name} "${default_var}" PARENT_SCOPE)
        return()
    endif ()

    message(STATUS "[FLAG]: Flag ${flag_name} is manually set as ${${flag_name}}")
endfunction()

# Include CMake submodules
auto_set_flag(SUPRESS_DEBUG_INFO            "False")
auto_set_flag(DISABLE_APT_UPDATE            "False")
auto_set_flag(DISABLE_PYTHON3_VENV_SETUP    "False")
if ("${SUPRESS_DEBUG_INFO}" STREQUAL "True")
    add_compile_definitions(SUPRESS_DEBUG_INFO=1)
else ()
    add_compile_definitions(SUPRESS_DEBUG_INFO=0)
endif ()

set(LOADED_MODULE_LIST "")
include(cmake_modules/color_the_console.cmake)
include(cmake_modules/dependencies_auto_install.cmake)
include(cmake_modules/utests.cmake)

find_package(Python3 COMPONENTS Interpreter Development)
set(PythonVirtualEnvironment_DIR)
include(cmake_modules/python_venv.cmake)
console_turn_green()
message(STATUS "Python virtual environment is setup at `${PythonVirtualEnvironment_DIR}`")
console_reset_color()

include(cmake_modules/xxd_compiler.cmake)

console_turn_green()
message(STATUS "The following CMake modules are loaded:")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    message(STATUS "\t\t${Module}")
endforeach (Module IN LISTS LOADED_MODULE_LIST)
message(STATUS "\t")
console_reset_color()

# Universal include directory
include_directories(include)

add_compile_definitions("CMAKE_SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")
add_compile_definitions("CMAKE_BINARY_DIR=\"${CMAKE_BINARY_DIR}\"")

add_compile_options(-g3 -O0)

# ##1: Sysdarft debug library
add_library(sysdarft_debug STATIC
        include/debug.h
        src/debug/stack_frame_tracer.cpp
        src/debug/sysdarft_error.cpp
)
link_libraries(sysdarft_debug)

# ##2: Amber Screen Display
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" TRIMMED_Python3_VERSION "${Python3_VERSION}")
auto_set_flag(Pybind11_DIR "${PythonVirtualEnvironment_DIR}/lib/python${TRIMMED_Python3_VERSION}/site-packages/pybind11/share/cmake/pybind11")
list(APPEND CMAKE_PREFIX_PATH "${Pybind11_DIR}")
console_turn_yellow()
message(STATUS "pybind11 set to ${Pybind11_DIR}")
console_reset_color()
find_package(pybind11 REQUIRED)
set(PYBIND11_CPP_STANDARD -std=c++20)
link_libraries(pybind11::embed Python3::Python)
add_library(sysdarft_amber_phosphor_screen STATIC
    include/sysdarft_display.h
    src/display/sysdarft_display.cpp
)
link_libraries(sysdarft_amber_phosphor_screen)

# ##3: Resource Packer for script files and other resource files
add_compile_definitions(FUSE_FSNAME="SysdarftResourceFS")
auto_set_flag(FOURCE_FUSE_FOREGROUND_WITH_DEBUG_INFO "False")
if ("${FOURCE_FUSE_FOREGROUND_WITH_DEBUG_INFO}" STREQUAL "True")
    add_compile_definitions(FOURCE_FUSE_FOREGROUND_WITH_DEBUG_INFO=1)
else ()
    add_compile_definitions(FOURCE_FUSE_FOREGROUND_WITH_DEBUG_INFO=0)
endif ()
auto_set_flag(RESOURCE_PACK_TMP_DIR "/tmp/.Sysdarft/Resources")
add_compile_definitions(RESOURCE_PACK_TMP_DIR="${RESOURCE_PACK_TMP_DIR}")
find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 REQUIRED fuse)
add_library(sysdarft_resource_packer STATIC
        include/res_packer.h
        src/utils/res_packer/res_packer.cpp
        src/utils/res_packer/filesystem_operations.cpp
)
target_include_directories(sysdarft_resource_packer PUBLIC ${FUSE3_INCLUDE_DIRS})
target_link_libraries(sysdarft_resource_packer PUBLIC ${FUSE3_LIBRARIES})
target_compile_options(sysdarft_resource_packer PUBLIC ${FUSE3_CFLAGS_OTHER})
link_libraries(sysdarft_resource_packer)
sysdarft_xxd_link_library(sysdarft_resource_packer)

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

sysdarft_add_test(res_packer
    utests/res_packer_test.cpp
)
