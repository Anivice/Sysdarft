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

```
