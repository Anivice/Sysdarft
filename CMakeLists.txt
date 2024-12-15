cmake_minimum_required(VERSION 3.29)

project(Sysdarft C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
#        -D_FORTIFY_SOURCE=2                  # Buffer overflow detection on safer libc functions (e.g., memcpy).
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
        -lasan -lubsan
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
        -lasan -lubsan
)

add_compile_options(${compiler_options})
add_link_options(${linker_options})

# ccache can significantly speed up compiling process
find_program(CCACHE_PROGRAM ccache REQUIRED)
if(CCACHE_PROGRAM)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

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

# Debug:
add_library(sysdarft_debug SHARED
        src/debug/debug.cpp src/include/debug.h src/include/debug.inl)
add_unit_test(test.log tests/test.log.cpp sysdarft_debug)
add_unit_test(test.sysdarft_error tests/test.sysdarft_error.cpp sysdarft_debug)

# Message Map:
add_library(sysdarft_msg_map STATIC src/include/msg_map.h)
set_target_properties(sysdarft_msg_map PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(sysdarft_msg_map PUBLIC sysdarft_debug)
add_unit_test(test.msgmap tests/test.msgmap.cpp sysdarft_msg_map)

# Modular:
add_library(sysdarft_modular SHARED
        src/include/module.h src/modular/module.cpp)
target_link_libraries(sysdarft_modular PUBLIC sysdarft_msg_map sysdarft_debug)
add_unit_test(test.modular tests/test.modular.cpp sysdarft_modular)

add_subdirectory(modules)

# Config
add_library(sysdarft_config SHARED
        src/config/config.cpp)
target_link_libraries(sysdarft_config PUBLIC sysdarft_debug)
add_unit_test(test.config tests/test.config.cpp sysdarft_config)

add_custom_target(CopyConfigFiles ALL
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/tests/good_example.conf" "${CMAKE_CURRENT_BINARY_DIR}"
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/tests/bad_example.conf" "${CMAKE_CURRENT_BINARY_DIR}"
        VERBATIM
        DEPENDS tests/good_example.conf tests/bad_example.conf
        COMMENT "Copying config files..."
)
