<<<<<<< HEAD
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
=======
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
>>>>>>> SysdarftRemake/rebase

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
<<<<<<< HEAD
        -g3                                  # Maximum debug information, including macro expansions
<<<<<<< HEAD
        -O0                                  # No optimization, easier for debugging
        -fstack-usage                        # Generate stack usage info for each function
        -fstack-protector-all                # Protect all functions with a stack canary to prevent stack overflow attacks
        -D_FORTIFY_SOURCE=2                  # Buffer overflow detection on safer libc functions (e.g., memcpy)
        -gdwarf-4
=======
=======
        -g3                                     # Maximum debug information, including macro expansions
>>>>>>> SysdarftRemake/ugly
        -O0
        -fstack-usage                           # Generate stack usage info for each function
        -fstack-protector-all                   # Protect all functions with a stack canary to prevent stack overflow attacks
        # -D_FORTIFY_SOURCE=2                   # Buffer overflow detection on safer libc functions (e.g., memcpy).
        # You need to enable optimization for _FORTIFY_SOURCE to work!
        -gdwarf-4                               # Generate DWARF version 4 debug information

        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer
<<<<<<< HEAD
>>>>>>> SysdarftRemake/rebase

        # Sanitize memory and thread issues
        -fsanitize=address                   # Detect illegal memory access such as buffer overflows and use-after-free
        -fsanitize=undefined                 # Detect undefined behavior like integer overflows and null dereferencing
<<<<<<< HEAD
        #        -fsanitize=thread                    # Ensure thread safety by detecting data races

        # Linker options
        #        -Wl,--verbose                        # Verbose linker output to show linking process details
        -flto                                # Enable Link-Time Optimization, but keep it compatible with debug builds
        -fprofile-arcs -ftest-coverage
=======
        # Uncomment if debugging threading issues:
        # -fsanitize=thread                   # Ensure thread safety by detecting data races

        # Code coverage options
        -fprofile-arcs                       # Enable code coverage instrumentation
        -ftest-coverage                      # Generate coverage test data
=======
        -std=c++23
>>>>>>> SysdarftRemake/ugly
        -lasan -lubsan -fPIC --pie
>>>>>>> SysdarftRemake/rebase
)

set(linker_options
        # Linker options for memory safety, thread safety, and verbose debugging
<<<<<<< HEAD
        -fuse-ld=gold                           # Use the gold linker for faster linking (optional, depending on system availability)
=======
>>>>>>> SysdarftRemake/rebase
        -Wl,--no-omagic                         # Prevent the generation of object files in memory; useful for debugging
        -Wl,--as-needed                         # Only link libraries that are actually needed to reduce binary size
        -Wl,--fatal-warnings                    # Treat all linker warnings as errors to catch issues early
        -Wl,-z,relro                            # Read-only relocations to prevent certain memory exploits (optional)
        -Wl,-z,now                              # Fully resolve all symbols during the link time for extra safety
        -Wl,-z,noexecstack                      # Prevent execution of code on the stack (security hardening)
        -Wl,-z,defs                             # Ensure all symbols are defined, and prevent undefined symbols
<<<<<<< HEAD
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
else ()
    add_compile_options(-g3 -O0 -flto -fPIC)
    add_link_options(-g3 -O0 -flto -fPIC)
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

# Tools
add_library(sysdarft_tools SHARED
        include/tools.h
        src/tools/obtain_xxd_lib_path.cpp)
target_link_libraries(sysdarft_tools PUBLIC sysdarft_debug)
sysdarft_add_test(get_xxd_lib_path utests/get_lib_path_test.cpp sysdarft_tools)

# Event Vectors
add_library(sysdarft_event_vec SHARED
        include/event_vector.h
        include/pygame_keys.h
        src/evet_vec/evet_vector.cpp)

# Amber Screen Display
string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*" "\\1" TRIMMED_Python3_VERSION "${Python3_VERSION}")
auto_set_flag(Pybind11_DIR "${PythonVirtualEnvironment_DIR}/lib/python${TRIMMED_Python3_VERSION}/site-packages/pybind11/share/cmake/pybind11")
list(APPEND CMAKE_PREFIX_PATH "${Pybind11_DIR}")

find_package(pybind11 REQUIRED)
add_library(sysdarft_amber_phosphor_screen SHARED
        include/sysdarft_display.h
        src/display/sysdarft_display.cpp
)
target_link_libraries(sysdarft_amber_phosphor_screen PUBLIC pybind11::embed Python3::Python sysdarft_resource_packer sysdarft_debug sysdarft_tools sysdarft_event_vec)
sysdarft_add_test(sysdarft_display utests/sysdarft_display_test.cpp "sysdarft_resource_packer;sysdarft_amber_phosphor_screen;sysdarft_debug;sysdarft_tools;sysdarft_event_vec")

# Amber Phosphor Screen Unit Test
sysdarft_add_test(amber_phosphor_screen_test utests/amber_phosphor_screen_test.cpp "sysdarft_resource_packer;sysdarft_debug;pybind11::embed;Python3::Python;sysdarft_tools;sysdarft_event_vec")

# Mount Project Resource Utility
add_executable(mount.prjres utils/mount_project_resource.cpp include/fuse_operations.h)
target_link_libraries(mount.prjres PUBLIC sysdarft_resource_packer)

# Sound support
find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSE REQUIRED libpulse-simple libpulse)
add_library(sysdarft_speaker SHARED
        include/sysdarft_speaker.h
        src/speaker/sysdarft_speaker.cpp
)
target_include_directories(sysdarft_speaker PUBLIC ${PULSE_INCLUDE_DIRS})
target_link_libraries(sysdarft_speaker PUBLIC sysdarft_debug ${PULSE_LIBRARIES})
sysdarft_add_test(sysdarft_speaker utests/sysdarft_sound_test.cpp "sysdarft_resource_packer;sysdarft_speaker")

# Automatically update README.md
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/scripts/generate_readme.bash "${CMAKE_SOURCE_DIR}")
=======
        -Wl,-O0

        -gdwarf-4                               # Generate detailed debug information for the linker
        -fno-eliminate-unused-debug-types
        -fno-omit-frame-pointer

        # Stack protection
        -fstack-protector-all                   # Link with stack protection for all functions
        -std=c++23
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
        __CLEAN_OUTPUT__
)

# Define include directories globally (can be refined per target)
include_directories(src/include)

# Debug:
add_library(SysdarftDebug OBJECT
        src/SysdarftDebug.cpp
        src/include/SysdarftDebug.h
        src/include/SysdarftDebug.inl
        src/ProprocessingFlagChecker.cpp
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

# Curses:
find_package(Curses REQUIRED)
add_library(SysdarftCursesUI OBJECT src/SysdarftCursesUI.cpp)
target_include_directories(SysdarftCursesUI PUBLIC src/include)
target_link_libraries(SysdarftCursesUI PUBLIC ${CURSES_LIBRARIES})

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
)
target_include_directories(SysdarftCoding PUBLIC src/include)

# I/O Controller Hub
add_library(SysdarftICH OBJECT
        src/include/SysdarftIOHub.h
        src/SysdarftIOHub.cpp
        src/include/SysdarftHardDisk.h
        src/ext_dev/SysdarftHardDisk.cpp
)
target_include_directories(SysdarftICH PUBLIC src/include)

# Sysdarft Shared Library:
add_library(Sysdarft SHARED src/include/WorkerThread.h)
target_include_directories(Sysdarft PUBLIC src/include)
target_link_libraries(Sysdarft PUBLIC
        SysdarftDebug
        SysdarftMessageMap
        SysdarftModule
        SysdarftGlobalEvents
        SysdarftCursesUI
        SysdarftCoding
        SysdarftCPU
        SysdarftICH
)

# Enable testing
enable_testing()

# Function to add unit tests
function(add_unit_test TARGET_NAME TEST_FILE)
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TARGET_NAME} PUBLIC Sysdarft)
    add_test(NAME "Unit Test: ${TARGET_NAME}"
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/wrpExpect.sh 
            ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME})
endfunction()

add_unit_test(test.error tests/test.error.cpp)
add_unit_test(test.log tests/test.log.cpp)
add_unit_test(test.execmd tests/test.execmd.cpp)
add_unit_test(test.curses tests/test.curses.cpp)

add_library(ExampleModule SHARED tests/example_module.cpp)
add_unit_test(test.module tests/test.module.cpp)
add_dependencies(test.module ExampleModule)

add_unit_test(test.message_map tests/test.message_map.cpp)
add_unit_test(test.coding tests/test.coding.cpp)
add_unit_test(test.register tests/test.register.cpp)
add_unit_test(test.memory tests/test.memory.cpp)
set_tests_properties("Unit Test: test.memory" PROPERTIES LABELS "ThreadSanitizerExclude")
add_unit_test(test.operand tests/test.operand.cpp)
add_unit_test(test.decode tests/test.decode.cpp)
add_unit_test(test.executor tests/test.executor.cpp)
add_unit_test(test.arithmetic tests/test.arithmetic.cpp)
add_unit_test(test.lgAbit tests/test.lgAbit.cpp)
add_unit_test(test.dataTsf tests/test.dataTsf.cpp)
add_unit_test(test.assembler tests/test.assembler.cpp)
add_unit_test(test.controlflow tests/test.controlflow.cpp)
add_unit_test(test.io tests/test.io.cpp)
add_unit_test(test.preprocessor tests/test.preprocessor.cpp)

add_custom_target(
        COPY_HDA_FILE ALL
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/tests/hda.img ${CMAKE_CURRENT_BINARY_DIR}/
        COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/tests/example.sysasm ${CMAKE_CURRENT_BINARY_DIR}/
        DEPENDS tests/hda.img tests/example.sysasm
        COMMENT "Copy disk image sample with ASCII texts in it."
)

# Console Executable:
<<<<<<< HEAD
#add_executable(sysdarft-system src/main.cpp)
#target_link_libraries(sysdarft-system PUBLIC sysdarft)

#link_libraries(sysdarft)

# Modules
#add_subdirectory(gui_module)
>>>>>>> SysdarftRemake/rebase
=======
add_executable(sysdarft-system src/SysdarftMain.cpp)
target_link_libraries(sysdarft-system PUBLIC Sysdarft)
>>>>>>> SysdarftRemake/ugly
