# Sysdarft
![Lines of Code](https://img.shields.io/badge/ProjectLines-45138-cyan)
![Size of Code](https://img.shields.io/badge/ProjectSize-2072%20K-yellow)
[![Automated CMake Test Workflow](https://github.com/Anivice/Sysdarft/actions/workflows/CMake_GitHub_Action.yml/badge.svg)](https://github.com/Anivice/Sysdarft/actions/workflows/CMake_GitHub_Action.yml)

<!--
    Sysdarft.md
    
    Copyright 2025 Anivice Ives
    
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    SPDX-License-Identifier: GPL-3.0-or-later
-->

Sysdarft is a hypothetical binary 64bit architecture.
It aims to have an ASM compiler, an invented high level language (higher than ASM, that is),
debugger, emulator (ncurses only, extendable if desired),
and an Operating System Kernel (Somewhat similar to DOS, since it has no protected mode).
It aims to be straightforward.

## How to Build

### Requirements

For Debian:

```shell
    sudo apt-get update
    sudo apt-get install -y \
        ccache \
        build-essential \
        libncurses-dev \
        bsdutils \
        libboost-dev \
        libboost-filesystem-dev \
        libboost-thread-dev \
        nlohmann-json3-dev \
        libcurl4-openssl-dev \
        expect \
        bc \
        software-properties-common \
        wget \
        apt-transport-https \
        gnupg cmake libsfml-dev libasio-dev
```

For Fedora:

```shell
    sudo dnf install -y \
        ccache \
        gcc-c++ \
        ncurses-devel \
        bsdmainutils \
        boost-devel \
        boost-filesystem-devel \
        boost-thread-devel \
        nlohmann-json-devel \
        libcurl-devel \
        expect \
        bc \
        dnf-plugins-core \
        wget \
        gnupg2 \
        cmake \
        sfml-devel \
        asio-devel
```

### Build

Normal build process:

```shell
    mkdir build && cd build && cmake .. && make -j $(nproc)
```

Or, if you wish to build the AppImage instead:

```shell
    mkdir build && cd build && cmake .. && make AppImage -j $(nproc)
```

Or, if you wish to build a single executable
(but external libraries are still dynamically linked)
but **not** an AppImage:

```shell
    mkdir build && cd build && cmake .. -D STATIC_BUILD=True && make AppImage -j $(nproc)
```

All will generate a target executable `sysdarft-system`.

### Command Help:

```shell
Usage: sysdarft-system [OPTIONS]
Options:
    -h, --help               Show this help message
    -v, --version            Output version information
    -m, --module <arg>       Load a module
                                 This option can be used multiple times
                                 to load multiple modules
    -V, --verbose            Enable verbose mode
    -c, --compile <arg>      Compile a file
                                 This option can be used multiple times
                                 to compile multiple files into one single binary
    -o, --output <arg>       Compilation output file
    -f, --format <arg>       Compile format. It can be bin, exe, or sys
    -I, --include <arg>      Specify one or more include path
    -R, --regex              If .equ preprocessor will be using regular expression
                                 If this option is not set, .equ will simply replace the string
                                 If this option is set, .equ will be processed
                                 using `/usr/bin/sed -E 's/../../g'`
                                 This will, of course, introduce performance downgrade when compiling
    -d, --disassem <arg>     Disassemble a file
    -g, --origin <arg>       Redefine origin for disassembler
                                 When left unset, origin is 0
    -b, --bios <arg>         Specify a BIOS firmware binary
    -L, --hdd <arg>          Specify a Hard Disk
    -A, --fda <arg>          Specify floppy disk A
    -B, --fdb <arg>          Specify floppy disk B
    -M, --memory <arg>       Specify memory size (in MB)
                                 Left unset and the default size is 32MB
    -S, --boot               Boot the system
    -D, --debug <arg>        Boot the system with remote debug console
                                 The system will not be started unless the debug console is connected
                                 Disconnecting debug console will cause system to immediately halt,
                                 which is equivalent to pulling the plug
                                 Debug server expects this argument: [Debug Server IP Address]:[Port]
    -N, --no-curses          Disable Curses, and use console output (will cause format issues)
    -W, --with-gui           Launch GUI display
    -F, --font <arg>         Specify a font name or a font file
                                 Use `--font help` to list them
    -C, --crow-log <arg>     Specify the log file of crow service
```

For more details, please build `SYSDARFT SOFTWARE DEVELOPMENT MANUAL` using

```shell
    make Sysdarft.pdf
```

You will need a full set of LaTeX tools on your system.
