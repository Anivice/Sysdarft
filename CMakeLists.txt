cmake_minimum_required(VERSION 3.29)

project(Sysdarft C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 20)

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
        -O1
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
        # -fsanitize=thread                       # Link the ThreadSanitizer runtime for thread safety
        -fsanitize=undefined                    # Link the UndefinedBehaviorSanitizer for detecting undefined behavior

        # Stack protection and LTO options
        -fstack-protector-all                   # Link with stack protection for all functions
        -flto                                   # Enable link-time optimization (can be disabled during debug for clarity)
        -fprofile-arcs -ftest-coverage
)

add_compile_options(${compiler_options})
add_link_options(${linker_options})

enable_testing()

function(add_unit_test
        TARGET_NAME
        TEST_FILE
        LINKED_LIBRARIES
)
    add_executable(${TARGET_NAME} ${TEST_FILE})

    foreach(lib IN LISTS LINKED_LIBRARIES)
        target_link_libraries(${TARGET_NAME} PUBLIC ${lib})
        add_dependencies(${TARGET_NAME} ${lib})
    endforeach()

    add_test(NAME "unit_test_${TARGET_NAME}"
            COMMAND ${CMAKE_BINARY_DIR}/${TARGET_NAME}
    )
endfunction()

include_directories(src/include)

add_library(sysdarft_debug SHARED
    src/debug/debug.cpp src/include/debug.h
)
add_unit_test(test.log tests/test.log.cpp sysdarft_debug)
add_unit_test(test.sysdarft_error tests/test.sysdarft_error.cpp sysdarft_debug)
