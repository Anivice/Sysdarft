# CMake modules

## Introduction

This is the introduction for CMake modules located under this
directory. It will explain their use. We currently have
* [check_program](#check_program)
* [color_the_console](#color_the_console)
* [dependencies_auto_install](#dependencies_auto_install)
* [python_venv](#python_venv)
* [utests (Unit Tests)](#utests)
* [xxd_compiler](#xxd_compiler)

## General environment variables

 | Variable                      | Explanation                                                                      |
 |-------------------------------|----------------------------------------------------------------------------------|
 | `SUPRESS_DEBUG_INFO`          | Supress debug information                                                        |
 | `DISABLE_APT_UPDATE`          | Disable `apt` update check                                                       |
 | `DISABLE_PYTHON3_VENV_SETUP`  | Disable Python 3 virtual environment set up                                      |
 | `LOADED_MODULE_LIST`          | A list of all loaded modules, update automatically when loading the CMake module |

## check_program
check_program module provides function `check_program(prog_name prog_exec)`.
It will automatically find required programs and define a 
marco named `${prog_exec}_EXECUTABLE`, which will indicate
the program's path.

If the program is not present in the
system, i.e., not found in `PATH`, CMake will generate an
error. Also, `check_program` will check simple usability
of the program by executing the program appending `--version`
parameter. Detailed output by this command will be printed
by CMake as `NOTICE` level, which can be suppressed by setting
marco `SUPRESS_DEBUG_INFO` to `True`.

## color_the_console
`bash` is required for this module to work. `color_the_console`
provides the following functions:

| Function Name             | Purpose                                                  |
|---------------------------|----------------------------------------------------------|
| `console_turn_red`        | Changes the console text color to red (`\033[31m`).      |
| `console_turn_dim`        | Dims the console text (`\033[2m`).                       |
| `console_turn_blue`       | Changes the console text color to blue (`\033[34m`).     |
| `console_turn_green`      | Changes the console text color to green (`\033[32m`).    |
| `console_turn_yellow`     | Changes the console text color to yellow (`\033[33m`).   |
| `console_reset_color`     | Resets the console text color to default (`\033[0m`).    |

## dependencies_auto_install
Loading this module will automatically install dependencies using
package manager. Currently, it supports Debian/Ubuntu and Fedora.

For development purposes, `apt` update procedure can be disable
by setting marco `DISABLE_APT_UPDATE` to `True`

Setting marco `SUPRESS_DEBUG_INFO` to `True` will disable 
output from the package manager.

## python_venv
Loading this module will automatically generate python virtual
environment and install required dependencies. The virtual
environment this module setup in will be marked in marco
`PythonVirtualEnvironment_DIR`, **be sure to define this marco
before loading the module by `set(PythonVirtualEnvironment_DIR)`,
otherwise, the module would not be able to set this marco
in its parent scope**.

Output from `pip` can be suppressed by setting marco
`SUPRESS_DEBUG_INFO` to `True`.

For development purposes, virtual environment creation can
be skipped by setting marco `DISABLE_PYTHON3_VENV_SETUP`
to `True`.

## utests
This module provides the function `sysdarft_add_test(TEST_NAME TEST_FILES)`,
which will automatically create a test called `Sysdarft_test_case_${TEST_NAME}`

## xxd_compiler
This module provides a way to compile resource file into
a `.cpp` file under `${CMAKE_BINARY_DIR}/res`, and generate
a `${CMAKE_BINARY_DIR}/include/res/resource_file_list.h`,
which provides **extern** references to resource file
content (i.e., resource file content in `unsigned char *`,
and its length in `unsigned int`). The max file size for
a single file is **4GB**. since it uses 32bit file length identifier.

### Usage explanation

#### Function definition
The function this module provides is
`sysdarft_add_xxd_target(TARGET_NAME TARGET_FILE PARENT_DEPENDENCY_NAME LIBRARY_LIST)`

 | Paramater     | Explanation                                                |
 |---------------|------------------------------------------------------------|
 | `TARGET_NAME` | CMake target name                                          |
 | `TARGET_FILE` | Path to file that wishes to be compiled into resource file |
 | `HOST_NAME`   | CMake target name that wish to link to the binary library  | 


#### Usage example
```cmake
cmake_minimum_required(VERSION 3.30)

project(host_executable C CXX)

set(LOADED_MODULE_LIST "")
include(cmake_modules/xxd_compiler.cmake)

add_executable(host_executable main.cpp)
sysdarft_add_xxd_target(ResourcePack res/resource.res host_executable)
```
