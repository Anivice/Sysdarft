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

# WARNING:  Standard C++14 library can cause thread sanitizers to report data race in certain areas,
#           which is considered a false positive, but will cause the unit test to fail. In C++23 this
#           issue simply does not exist

# Universal compiler and linker flags
set(compiler_options
        # Warnings and diagnostics
        -Wall                                   # Enable common warnings
        -Wextra                                 # Enable extra warnings
        -Wpedantic                              # Strict compliance with the standard
        -Wunused                                # Warn about unused variables, functions, etc.
        -Wuninitialized                         # Warn if variables are used uninitialized
        -fdiagnostics-show-option               # Show which option triggered the warning
        -fdiagnostics-color=always              # Enable colored diagnostics for better readability

        # Debugging and stack protection
        -g3                                     # Maximum debug information, including macro expansions
        -O0
        -fstack-usage                           # Generate stack usage info for each function
        -fstack-protector-all                   # Protect all functions with a stack canary to prevent stack overflow attacks
        # -D_FORTIFY_SOURCE=2                   # Buffer overflow detection on safer libc functions (e.g., memcpy).
        # You need to enable optimization for _FORTIFY_SOURCE to work!
        -gdwarf-4                               # Generate DWARF version 4 debug information

        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer
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

        # Stack protection
        -fstack-protector-all                   # Link with stack protection for all functions
        -lasan -lubsan -fPIC --pie --whole-file
)

if ("${COMPILE_WITH_MEMORY_SANITIZERS}" STREQUAL "True")
    message(STATUS "Sanitizers for memory enabled")
    list(APPEND compiler_options
            -fsanitize=address                  # Detect illegal memory access such as buffer overflows and use-after-free
            -fsanitize=undefined                # Detect undefined behavior like integer overflows and null dereferencing
            # Code coverage options
            -fprofile-arcs                      # Enable code coverage instrumentation
            -ftest-coverage                     # Generate coverage test data
    )

    list(APPEND linker_options
            -fsanitize=address                  # Link the AddressSanitizer runtime for memory integrity
            -fsanitize=undefined                # Link the UndefinedBehaviorSanitizer for detecting undefined behavior
            # Code coverage options
            -fprofile-arcs                      # Enable code coverage instrumentation
            -ftest-coverage                     # Generate coverage test data
    )
endif ()

if ("${COMPILE_WITH_THREAD_SANITIZERS}" STREQUAL "True")
    message(STATUS "Sanitizers for thread enabled")
    list(APPEND compiler_options
        -fsanitize=undefined                    # Detect undefined behavior like integer overflows and null dereferencing
        -fsanitize=thread                       # Ensure thread safety by detecting data races
    )
    list(APPEND linker_options
        -fsanitize=undefined                    # Link the UndefinedBehaviorSanitizer for detecting undefined behavior
        -fsanitize=thread                       # Link the ThreadSanitizer runtime for thread safety
    )
endif ()

# Sanity check
if (("${COMPILE_WITH_MEMORY_SANITIZERS}" STREQUAL "True") AND ("${COMPILE_WITH_THREAD_SANITIZERS}" STREQUAL "True"))
    message(FATAL_ERROR "Memory sanitizers and thread sanitizers are mutually exclusive!")
endif ()

set(optimization_flags
        -O3
        -march=native
        -mtune=native
        -flto
        -fomit-frame-pointer
        -ffast-math
        -fstrict-aliasing
        -fdata-sections
        -ffunction-sections
        -D_FORTIFY_SOURCE=2
        -s
)

set(optimization_link_flags
        -O3
        -march=native
        -mtune=native
        -flto
        -fomit-frame-pointer
        -ffast-math
        -fstrict-aliasing
        -fdata-sections
        -ffunction-sections
        -Wl,--gc-sections
        -D_FORTIFY_SOURCE=2
        -s
)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_options(${compiler_options})
    add_link_options(${linker_options})
else ()
    add_compile_options(${optimization_flags})
    add_compile_options(${optimization_link_flags})
endif ()

# Read and process the information file
set(information_file "${CMAKE_CURRENT_SOURCE_DIR}/sysdarft_info.txt")
set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${information_file}")
file(READ "${information_file}" SYSDARFT_INFO_CONTENT)
string(REPLACE "\n" "\\n" SYSDARFT_INFO_CONTENT_ESCAPED "${SYSDARFT_INFO_CONTENT}")

add_compile_definitions(
        SYSDARFT_VERSION="0.0.1-Alpha"
        SYSDARFT_INFORMATION="${SYSDARFT_INFO_CONTENT_ESCAPED}"
        __CLEAN_OUTPUT__
)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_compile_definitions(__DEBUG__)
endif ()

# Define include directories globally (can be refined per target)
include_directories(src/include)

# Debug:
add_library(SysdarftDebug OBJECT
        src/SysdarftDebug.cpp
        src/include/SysdarftDebug.h
        src/include/SysdarftDebug.inl
        src/PreProcessingFlagChecker.cpp
        src/demangle.cpp
)
target_include_directories(SysdarftDebug PUBLIC src/include)

# Message Map:
add_library(SysdarftMessageMap OBJECT src/SysdarftMessageMap.cpp)
target_include_directories(SysdarftMessageMap PUBLIC src/include)

# Modular:
add_library(SysdarftModule OBJECT src/SysdarftModule.cpp)
target_include_directories(SysdarftModule PUBLIC src/include)

add_library(SysdarftGlobalEvents OBJECT src/GlobalEvents.cpp)
target_include_directories(SysdarftGlobalEvents PUBLIC src/include)

# Audio files
find_package(ZLIB REQUIRED)
add_executable(compress utils/compress.c src/coding/zlib.c)
target_link_libraries(compress PRIVATE ZLIB::ZLIB)

add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/resources
        COMMAND rm -rf bell.wav
        COMMAND cp "${CMAKE_CURRENT_SOURCE_DIR}/resources/bell.wav" bell.wav
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/compress
                "${CMAKE_CURRENT_SOURCE_DIR}/resources/bell.wav"
                "${CMAKE_CURRENT_BINARY_DIR}/resources/bell.bin"
        COMMAND xxd -n bell_sound -i "${CMAKE_CURRENT_BINARY_DIR}/resources/bell.bin" > ${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c
        COMMAND echo >> "${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c"
        COMMAND echo -n "unsigned long long int bell_sound_original_len = " >> "${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c"
        COMMAND stat --format=\"%s\" "${CMAKE_CURRENT_SOURCE_DIR}/resources/bell.wav" >> "${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c"
        COMMAND echo "\;" >> "${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c"
        DEPENDS resources/bell.wav compress
        COMMENT "Building audio resources.."
)

# Curses:
find_package(Curses REQUIRED)
find_package(SFML COMPONENTS audio system REQUIRED)
add_library(SysdarftCursesUI OBJECT
        src/SysdarftCursesUI.cpp
        src/gui/TerminalDisplay.cpp
        src/include/TerminalDisplay.hpp
        src/KeyControl.c
        src/include/ASCIIKeymap.h
        ${CMAKE_CURRENT_BINARY_DIR}/resources/bell.c)
target_include_directories(SysdarftCursesUI PUBLIC src/include)
target_link_libraries(SysdarftCursesUI PUBLIC ${CURSES_LIBRARIES}
        sfml-audio
        sfml-system
        ZLIB::ZLIB
        sfml-graphics
        sfml-window)

# CPU
add_library(SysdarftCPU OBJECT
        src/include/SysdarftRegister.h
        src/include/SysdarftMemory.h
        src/cpu/SysdarftMemory.cpp
        src/include/SysdarftCPUDecoder.h
        src/cpu/SysdarftCPUDecoder.cpp
        src/include/SysdarftInstructionExec.h
        src/cpu/SysdarftInstructionExec.cpp
        src/cpu/OutputCurrentContext.cpp
        src/cpu/Operations/Helper.cpp
        src/cpu/Operations/Arithmetic.cpp
        src/cpu/Operations/Misc.cpp
        src/cpu/Operations/DataTransfer.cpp
        src/cpu/Operations/LogicalAndBitwise.cpp
        src/cpu/Operations/ControlFlow.cpp
        src/include/SysdarftIOHub.h
        src/cpu/SysdarftCPUInterruption.cpp
        src/include/SysdarftCPU.h
        src/cpu/Operations/IOH.cpp
        src/cpu/SysdarftCPU.cpp
)
target_include_directories(SysdarftCPU PUBLIC src/include src/cpu/include)
set_target_properties(SysdarftCPU PROPERTIES LINKER_LANGUAGE CXX)

# Encoder and decoder
add_library(SysdarftCoding OBJECT
        src/coding/EncodeTarget.cpp
        src/coding/DecodeTarget.cpp
        src/coding/EncodeInstruction.cpp
        src/coding/DecodeInstruction.cpp
        src/coding/Assembler.cpp
        src/coding/PreProcessor.cpp
        src/coding/Linker.cpp
        src/coding/OperandSanity.cpp
        src/coding/HeadersAndDefinitions.cpp
        src/coding/zlib.c
)
target_include_directories(SysdarftCoding PUBLIC src/include)

# I/O Controller Hub
add_library(SysdarftICH OBJECT
        src/include/SysdarftIOHub.h
        src/SysdarftIOHub.cpp
        src/include/SysdarftDisks.h
        src/ext_dev/SysdarftBlockDevices.cpp
        src/include/SysdarftDisks.inl
        src/include/RealTimeClock.h
        src/ext_dev/RealTimeClock.cpp
)
target_include_directories(SysdarftICH PUBLIC src/include)

# Sysdarft Shared Library:
add_library(Sysdarft SHARED src/include/WorkerThread.h)
target_include_directories(Sysdarft PUBLIC src/include)
target_link_libraries(Sysdarft PRIVATE
        SysdarftDebug
        SysdarftMessageMap
        SysdarftModule
        SysdarftGlobalEvents
        SysdarftCursesUI
        SysdarftCoding
        SysdarftCPU
        SysdarftICH
)

# Console Executable:
find_package(nlohmann_json REQUIRED)
## Find the Asio headers
find_path(ASIO_INCLUDE_DIR
        NAMES asio.hpp
        PATHS /usr/include /usr/local/include
        doc "Path to the standalone Asio headers (asio.hpp)"
)
if(NOT ASIO_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find <asio.hpp>.")
else ()
    message(STATUS "Found <asio.hpp> in ${ASIO_INCLUDE_DIR}")
endif()

add_compile_definitions(ASIO_STANDALONE)

add_executable(sysdarft-system
        src/SysdarftConsole/SysdarftMain.cpp
        src/include/SysdarftMain.h
        src/SysdarftConsole/MainArgProcessor.cpp
        src/SysdarftConsole/MainCompile.cpp
        src/SysdarftConsole/RemoteDebugServer.cpp
        src/SysdarftConsole/ConditionalExpression.cpp
        src/SysdarftConsole/ShowContext.cpp
        src/SysdarftConsole/DebugAction.cpp
        src/SysdarftConsole/CrowLog.cpp
        src/SysdarftConsole/DebugConsole.cpp
        src/SysdarftConsole/debugger_operand.h
        src/SysdarftConsole/debugger_operand.cpp
        src/SysdarftConsole/Action.cpp
        src/SysdarftConsole/Continue.cpp
        src/SysdarftConsole/CrowShowContext.cpp
        src/SysdarftConsole/IntAlertSSE.cpp
        src/SysdarftConsole/IsAPIAvailable.cpp
        src/SysdarftConsole/SetBreakpoint.cpp
        src/SysdarftConsole/ShowBreakpoint.cpp
        src/SysdarftConsole/Stepi.cpp
        src/SysdarftConsole/Watcher.cpp
        src/SysdarftConsole/logo.c
        src/SysdarftConsole/DisassembleAnArea.cpp
        src/SysdarftConsole/PullData.cpp
)
target_include_directories(sysdarft-system PUBLIC ${ASIO_INCLUDE_DIR} src/include src/include/crow)
target_link_libraries(sysdarft-system PRIVATE Sysdarft nlohmann_json::nlohmann_json)

# Enable testing
enable_testing()

# Function to add unit tests
function(add_unit_test TARGET_NAME)
    set(ARGUMENTS "")
    foreach (FILE IN LISTS ARGN)
        LIST(APPEND ARGUMENTS -c "${CMAKE_CURRENT_SOURCE_DIR}/${FILE}")
    endforeach ()

    add_test(NAME "Compile Test: ${TARGET_NAME}"
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sysdarft-system ${ARGUMENTS}
                                                                -o ${TARGET_NAME}.bin
                                                                -f bin
                                                                -I ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )

    add_test(NAME "Symbol Extraction Test: ${TARGET_NAME}"
            COMMAND ${CMAKE_CURRENT_BINARY_DIR}/sysdarft-system ${ARGUMENTS}
                                                                -o ${TARGET_NAME}.sys
                                                                -f sys
                                                                -I ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
endfunction()

# Unit Tests:
add_unit_test(disk_io tests/disk_io.asm)
add_unit_test(rtc tests/rtc.asm)
add_unit_test(thread tests/thread.asm)

add_custom_target(
        COPY_SRC_FILE ALL
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/tests/hda.img ${CMAKE_CURRENT_BINARY_DIR}/
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/hda.img
        COMMENT "Copying assembly source code..."
)

if (NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    add_custom_target(STRIP_ALL ALL
            COMMAND strip sysdarft-system
            COMMAND strip libSysdarft.so
            DEPENDS sysdarft-system Sysdarft
            COMMENT "Stripping symbols...")
endif ()

# Documentation
function(add_pdf_target TARGET FILE)
    add_custom_target(
            ${TARGET}
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/convert.py
                    ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}
                    ${CMAKE_CURRENT_BINARY_DIR}/${TARGET}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc
            COMMENT "Generating pdf for ${FILE}..."
            DEPENDS ${FILE}
    )
endfunction()

add_pdf_target(Sysdarft.pdf doc/Sysdarft.md)

add_executable(keyEcho utils/keyEcho.c src/KeyControl.c)
