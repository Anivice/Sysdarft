cmake_minimum_required(VERSION 3.18)

# Setup language standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

project(Sysdarft C CXX)

# Platform check
if (NOT UNIX)
    message(FATAL_ERROR "Unsupported platform! This project is for Unix systems.")
endif()

# Build type specific definitions
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(SYSDARFT_TARGET_IS_DEBUG_BUILD)
    set(ENABLE_SANITIZERS TRUE)  # Enable sanitizers in Debug builds
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(SYSDARFT_TARGET_IS_RELEASE_BUILD)
    set(ENABLE_SANITIZERS FALSE)
endif()

# Check for GCC compiler
if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(FATAL_ERROR "This project requires GCC.")
endif()

# Function to auto-set flags
function(auto_set_flag flag_name default_var)
    set(${flag_name} "${${flag_name}}")
    if (NOT ${flag_name})
        message(STATUS "[FLAG]: Flag ${flag_name} is using default value: ${default_var}")
        set(${flag_name} "${default_var}" PARENT_SCOPE)
        return()
    endif()
    message(STATUS "[FLAG]: Flag ${flag_name} is manually set as ${${flag_name}}")
endfunction()

# Universal compiler and linker flags
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
        -fprofile-arcs -ftest-coverage
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
        -fprofile-arcs -ftest-coverage
)

# Include CMake submodules
auto_set_flag(SUPRESS_DEBUG_INFO "False")
if (SUPRESS_DEBUG_INFO STREQUAL "True")
    add_compile_definitions(SUPRESS_DEBUG_INFO=1)
endif()

set(LOADED_MODULE_LIST "")
include(cmake_modules/color_the_console.cmake)
auto_set_flag(DISABLE_APT_UPDATE "False")
include(cmake_modules/dependencies_auto_install.cmake)
include(cmake_modules/utests.cmake)

# Python3 virtual environment setup
set(PythonVirtualEnvironment_DIR "")
auto_set_flag(DISABLE_PYTHON3_VENV_SETUP "False")
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include(cmake_modules/python_venv.cmake)
message(STATUS "Python virtual environment is set up at '${PythonVirtualEnvironment_DIR}'")

include(cmake_modules/xxd_compiler.cmake)

console_turn_green()
message(STATUS "The following CMake modules are loaded:")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    message(STATUS "\t\t${Module}")
endforeach()
console_reset_color()

# Apply options conditionally for extreme debugging
check_program(bash BASH)
auto_set_flag(EXTREME_DEBUG_TRACE "False")
if (EXTREME_DEBUG_TRACE STREQUAL "True")
    set(LOCK_FILE "${CMAKE_BINARY_DIR}/.YOU_HAVE_BEEN_WARNED")
    if (NOT EXISTS "${LOCK_FILE}")
        message(WARNING
                "#############################################\n"
                "*   YOU HAVE ENABLED EXTREME DEBUG TRACE    *\n"
                "#############################################\n"
                "This feature is designed for debug purpose only and should *NOT* even be used in the testing stage! "
                "There will be *A LOG OF DEBUG INFORMATION* and tests are *NOT* guaranteed to work! "
                "Excessive lagging in response, unexpected file generation, extremely high resource usage will appear! "
                "\n** If you unexpectedly enabled this feature, press `Ctrl+C` to force exit the configuration process, "
                "set EXTREME_DEBUG_TRACE to \"False\" by pass `-D EXTREME_DEBUG_TRACE=\"False\"` to CMake, "
                "and re-initialize CMake again. **\n"
                "If you have a specific need for this feature or your head is so fucked up that there's trouble for you to "
                "even find Ctrl+C on your keyboard, proceed after press Enter, assuming you can still find that key in "
                "your current mental state. This warning message will appear ONLY ONCE!"
        )
        execute_process(COMMAND ${BASH_EXECUTABLE} -c "read -p 'Press [Enter] to continue...'")
        execute_process(COMMAND ${BASH_EXECUTABLE} -c "touch ${CMAKE_BINARY_DIR}/.YOU_HAVE_BEEN_WARNED")
    endif ()
    add_compile_options(${compiler_options})
    add_link_options(${linker_options})
endif()

# Universal include directory
include_directories(include)

add_compile_definitions(CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
add_compile_definitions(CMAKE_BINARY_DIR="${CMAKE_BINARY_DIR}")

# Sysdarft debug library
add_library(sysdarft_debug SHARED
        include/debug.h
        src/debug/stack_frame_tracer.cpp
        src/debug/sysdarft_error.cpp
)
sysdarft_add_test(cmake_sysdarft_test utests/cmake_sysdarft_test_case_test.cpp sysdarft_debug)

# Resource Packer
add_compile_definitions(FUSE_FSNAME="SysdarftResourceFS")
auto_set_flag(RESOURCE_PACK_TMP_DIR "/tmp/.Sysdarft/Resources")
add_compile_definitions(RESOURCE_PACK_TMP_DIR="${RESOURCE_PACK_TMP_DIR}")

find_package(PkgConfig REQUIRED)
pkg_check_modules(FUSE3 REQUIRED fuse)

add_library(sysdarft_resource_packer SHARED
        include/res_packer.h
        src/res_packer/res_packer.cpp
        src/res_packer/filesystem_operations.cpp
)
target_include_directories(sysdarft_resource_packer PUBLIC ${FUSE3_INCLUDE_DIRS})
target_link_libraries(sysdarft_resource_packer PUBLIC ${FUSE3_LIBRARIES} sysdarft_debug)
target_compile_options(sysdarft_resource_packer PUBLIC ${FUSE3_CFLAGS_OTHER})
sysdarft_xxd_link_library(sysdarft_resource_packer)
sysdarft_add_test(res_packer utests/res_packer_test.cpp "sysdarft_resource_packer")

# Amber Screen Display
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" TRIMMED_Python3_VERSION "${Python3_VERSION}")
auto_set_flag(Pybind11_DIR "${PythonVirtualEnvironment_DIR}/lib/python${TRIMMED_Python3_VERSION}/site-packages/pybind11/share/cmake/pybind11")
list(APPEND CMAKE_PREFIX_PATH "${Pybind11_DIR}")

find_package(pybind11 REQUIRED)
add_library(sysdarft_amber_phosphor_screen SHARED
        include/sysdarft_display.h
        src/display/sysdarft_display.cpp
)
target_link_libraries(sysdarft_amber_phosphor_screen PUBLIC pybind11::embed Python3::Python sysdarft_resource_packer sysdarft_debug)
sysdarft_add_test(sysdarft_display utests/sysdarft_display_test.cpp "sysdarft_resource_packer;sysdarft_amber_phosphor_screen;sysdarft_debug")

# Amber Phosphor Screen Unit Test
sysdarft_add_test(amber_phosphor_screen_test utests/amber_phosphor_screen_test.cpp "sysdarft_resource_packer;sysdarft_debug;pybind11::embed;Python3::Python")

# Mount Project Resource Utility
add_executable(mount.prjres utils/mount_project_resource.cpp include/fuse_operations.h)
target_link_libraries(mount.prjres PUBLIC sysdarft_resource_packer)

# Automatically update README.md
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/generate_readme.bash "${CMAKE_SOURCE_DIR}")
