# ccache_toolchain.cmake

# -----------------------------------------------------------------------------
# CMake Toolchain File for C and C++ with ccache Integration
# -----------------------------------------------------------------------------

# Specify the minimum CMake version required
cmake_minimum_required(VERSION 3.29)

# -----------------------------------------------------------------------------
# Compiler Definitions
# -----------------------------------------------------------------------------

# Specify the actual C compiler
# Change this path if you use a different compiler or it's located elsewhere
set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE STRING "C Compiler" FORCE)

# Specify the actual C++ compiler
# Change this path if you use a different compiler or it's located elsewhere
set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE STRING "C++ Compiler" FORCE)

# -----------------------------------------------------------------------------
# Compiler Launchers
# -----------------------------------------------------------------------------

# Ensure that ccache is installed and available in the PATH
find_program(CCACHE_PROGRAM ccache)
if(NOT CCACHE_PROGRAM)
    message(FATAL_ERROR "ccache not found! Please install ccache or adjust the toolchain file.")
endif()

# Set ccache as the compiler launcher for C
set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C Compiler Launcher" FORCE)

# Set ccache as the compiler launcher for C++
set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}" CACHE STRING "C++ Compiler Launcher" FORCE)

# -----------------------------------------------------------------------------
# Compiler Flags and Standards (Optional)
# -----------------------------------------------------------------------------

# Set the C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Position Independent Code (Optional)
# -----------------------------------------------------------------------------

# Enable position-independent code (useful for building shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -----------------------------------------------------------------------------
# Symbol Visibility (Optional)
# -----------------------------------------------------------------------------

# Set default visibility for symbols (useful for shared libraries)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# -----------------------------------------------------------------------------
# Additional CMake Settings (Optional)
# -----------------------------------------------------------------------------

# Prevent CMake from automatically adding '-rdynamic' on Unix-like systems
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "" CACHE STRING "Shared library link C flags" FORCE)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "" CACHE STRING "Shared library link CXX flags" FORCE)

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
        -O0
        -fstack-usage                        # Generate stack usage info for each function
        -fstack-protector-all                # Protect all functions with a stack canary to prevent stack overflow attacks
        #-D_FORTIFY_SOURCE=2                  # Buffer overflow detection on safer libc functions (e.g., memcpy).
        # You need to enable optimization for _FORTIFY_SOURCE to work!
        -gdwarf-4                            # Generate DWARF version 4 debug information

        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # Sanitize memory and thread issues
        -fsanitize=address                   # Detect illegal memory access such as buffer overflows and use-after-free
        -fsanitize=undefined                 # Detect undefined behavior like integer overflows and null dereferencing
        # Uncomment if debugging threading issues:
        # -fsanitize=thread                   # Ensure thread safety by detecting data races

        # Code coverage options
        -fprofile-arcs                       # Enable code coverage instrumentation
        -ftest-coverage                      # Generate coverage test data
        -lasan -lubsan -fPIC --pie
)

set(linker_options
        # Linker options for memory safety, thread safety, and verbose debugging
        -Wl,--no-omagic                         # Prevent the generation of object files in memory; useful for debugging
        -Wl,--as-needed                         # Only link libraries that are actually needed to reduce binary size
        -Wl,--fatal-warnings                    # Treat all linker warnings as errors to catch issues early
        -Wl,-z,relro                            # Read-only relocations to prevent certain memory exploits (optional)
        -Wl,-z,now                              # Fully resolve all symbols during the link time for extra safety
        -Wl,-z,noexecstack                      # Prevent execution of code on the stack (security hardening)
        -Wl,-z,defs                             # Ensure all symbols are defined, and prevent undefined symbols
        -Wl,-O0

        -gdwarf-4                               # Generate detailed debug information for the linker
        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # AddressSanitizer and UndefinedBehaviorSanitizer linking
        -fsanitize=address                      # Link the AddressSanitizer runtime for memory integrity
        -fsanitize=undefined                    # Link the UndefinedBehaviorSanitizer for detecting undefined behavior
        # Uncomment if debugging threading issues:
        # -fsanitize=thread                       # Link the ThreadSanitizer runtime for thread safety

        # Stack protection
        -fstack-protector-all                   # Link with stack protection for all functions

        # Code coverage options
        -fprofile-arcs                          # Enable code coverage instrumentation
        -ftest-coverage                         # Generate coverage test data
        -lasan -lubsan -fPIC --pie --whole-file
)

# Common C flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${compiler_options}")

# Common C++ flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${compiler_options} -std=c++23")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${linker_options}")

# -----------------------------------------------------------------------------
# End of Toolchain File
# -----------------------------------------------------------------------------
