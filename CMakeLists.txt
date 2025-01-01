cmake_minimum_required(VERSION 3.29)

project(Sysdarft C CXX)

# Enable Ccache support
find_program(CCACHE_PROGRAM ccache)
if(NOT CCACHE_PROGRAM)
    message(FATAL_ERROR "ccache not found! Please install ccache or adjust the configuration.")
endif()

# Set ccache as the compiler launcher before setting the compiler
set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")

# Optionally, set the C and C++ compilers if you need specific versions
# set(CMAKE_C_COMPILER "/usr/bin/gcc" CACHE STRING "C Compiler" FORCE)
# set(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE STRING "C++ Compiler" FORCE)

# Set C and C++ standards
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable position-independent code if needed
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Set visibility properties
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

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
        # -D_FORTIFY_SOURCE=2                  # Buffer overflow detection on safer libc functions (e.g., memcpy).
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

add_compile_options(${compiler_options})
add_link_options(${linker_options})

# Read and process the information file
set(information_file "${CMAKE_CURRENT_SOURCE_DIR}/sysdarft_info.txt")
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${information_file}")
file(READ "${information_file}" SYSDARFT_INFO_CONTENT)
string(REPLACE "\n" "\\n" SYSDARFT_INFO_CONTENT_ESCAPED "${SYSDARFT_INFO_CONTENT}")

add_compile_definitions(
        SYSDARFT_VERSION="0.0.1"
        SYSDARFT_INFORMATION="${SYSDARFT_INFO_CONTENT_ESCAPED}"
        __DEBUG__
)

# Define include directories globally (can be refined per target)
include_directories(src/include)

# Debug:
add_library(SysdarftDebug OBJECT src/SysdarftDebug.cpp)
target_include_directories(SysdarftDebug PUBLIC src/include)

# Message Map:
add_library(SysdarftMessageMap OBJECT src/MessageMap.cpp)
target_include_directories(SysdarftMessageMap PUBLIC src/include)

# Modular:
add_library(SysdarftModule OBJECT src/SysdarftModule.cpp)
target_include_directories(SysdarftModule PUBLIC src/include)

add_library(SysdarftGlobalEvents OBJECT src/GlobalEvents.cpp)
target_include_directories(SysdarftGlobalEvents PUBLIC src/include)

# Curses:
find_package(Curses REQUIRED)
add_library(SysdarftCursesUI OBJECT src/SysdarftCursesUI.cpp)
target_include_directories(SysdarftCursesUI PUBLIC src/include)
target_link_libraries(SysdarftCursesUI PUBLIC ${CURSES_LIBRARIES})

## CPU
#add_library(sysdarft_processor OBJECT
#        src/include/EncodingDecoding.h
#        src/include/InstructionSet.h
#        src/sysdarft_processor/include/register_def.h
#        src/sysdarft_processor/io/disk_hub.cpp
#        src/sysdarft_processor/io/io_hub.cpp
#        src/sysdarft_processor/io/external_device_hub.cpp
#        src/sysdarft_processor/cpu/memory_io.cpp
#        src/sysdarft_processor/include/io.h
#        src/sysdarft_processor/include/gpu.h
#        src/sysdarft_processor/include/int.h
#        src/sysdarft_processor/include/instance.h
#        src/sysdarft_processor/interruptions/interruptions.cpp
#        src/sysdarft_processor/cpu/instruction_executor.cpp
#        src/sysdarft_processor/cpu/protected_mode_target_access.cpp
#        src/sysdarft_processor/cpu/register.cpp
#        src/sysdarft_processor/cpu/real_mode_target_access.cpp
#        src/sysdarft_processor/cpu/operation_triggerer.cpp
#        src/sysdarft_processor/integrated_devices/integrated_gpu.cpp
#        src/sysdarft_processor/instantiation/prc_inst_n_init.cpp
#        src/sysdarft_processor/include/cpu.inl
#        src/sysdarft_processor/cpu/initialization.cpp
#        src/sysdarft_processor/cpu/operations/arithmetic.cpp
#        src/sysdarft_processor/cpu/operations/control_flow.cpp
#        src/sysdarft_processor/cpu/operations/data_transfer.cpp
#        src/sysdarft_processor/cpu/operations/fpu.cpp
#        src/sysdarft_processor/cpu/operations/io.cpp
#        src/sysdarft_processor/cpu/operations/logical_and_bitwise.cpp
#        src/sysdarft_processor/cpu/operations/miscellaneous.cpp
#        src/sysdarft_processor/cpu/operations/os.cpp
#        src/sysdarft_processor/cpu/decoder/decode.instruction.cpp
#        src/sysdarft_processor/cpu/decoder/decode.target.cpp
#        src/sysdarft_processor/cpu/output_debug_info.cpp
#        src/sysdarft_processor/cpu/operations/of_check.cpp
#)
#target_include_directories(sysdarft_processor PUBLIC src/include src/sysdarft_processor/include)

# Encoder and decoder
add_library(SysdarftCoding OBJECT
        src/coding/EncodeTarget.cpp
        src/coding/DecodeTarget.cpp
        src/coding/EncodeInstruction.cpp
        src/coding/DecodeInstruction.cpp
)
target_include_directories(SysdarftCoding PUBLIC src/include)

# Sysdarft Shared Library:
add_library(sysdarft SHARED src/include/WorkerThread.h)
target_include_directories(sysdarft PUBLIC src/include)
target_link_libraries(sysdarft PUBLIC
        SysdarftDebug
        SysdarftMessageMap
        SysdarftModule
        SysdarftGlobalEvents
        SysdarftCursesUI
        SysdarftCoding
#        sysdarft_processor
#        sysdarft_encoder
)

# Enable testing
enable_testing()

# Function to add unit tests
function(add_unit_test TARGET_NAME TEST_FILE)
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TARGET_NAME} PUBLIC sysdarft)
    add_test(NAME "unit_test_${TARGET_NAME}" COMMAND $<TARGET_FILE:${TARGET_NAME}>)
endfunction()

add_unit_test(test.error tests/test.error.cpp)
add_unit_test(test.log tests/test.log.cpp)
add_unit_test(test.curses tests/test.curses.cpp)

add_library(ExampleModule SHARED tests/example_module.cpp)
add_unit_test(test.module tests/test.module.cpp)
add_dependencies(test.module ExampleModule)

add_unit_test(test.message_map tests/test.message_map.cpp)
add_unit_test(test.coding tests/test.coding.cpp)

# Console Executable:
#add_executable(sysdarft-system src/main.cpp)
#target_link_libraries(sysdarft-system PUBLIC sysdarft)

#link_libraries(sysdarft)

# Modules
#add_subdirectory(gui_module)
