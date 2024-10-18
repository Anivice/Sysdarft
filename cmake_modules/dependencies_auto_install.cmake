set(MODULE_NAME "DependenciesAutoInstall")
set(LOADED_MODULE_LIST "${LOADED_MODULE_LIST}")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    if ("X${Module}" STREQUAL "X${MODULE_NAME}")
        return()
    endif ()
endforeach (Module IN LISTS LOADED_MODULE_LIST)
list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")

message(STATUS "CMake Module Dependencies Automatic Installation Loaded")

include("${CMAKE_CURRENT_LIST_DIR}/color_the_console.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")

check_program("grep" "GREP")
check_program("sudo" "SUDO")

# Detect the Linux distribution using /etc/os-release
execute_process(
        COMMAND ${GREP_EXECUTABLE} "^ID=" /etc/os-release
        OUTPUT_VARIABLE DISTRO
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

console_turn_blue()
message(STATUS "Installing dependencies...")
console_reset_color()

# Installation commands based on the detected distribution
if (DISTRO MATCHES "ubuntu|debian")
    console_turn_yellow()
    message(STATUS "Detected Ubuntu/Debian-based system")
    console_reset_color()

    console_turn_dim()
    if (NOT "${DISABLE_APT_UPDATE}" STREQUAL "True")
        if (${SUPRESS_DEBUG_INFO} STREQUAL "True")
            execute_process(COMMAND ${SUDO_EXECUTABLE} -E apt update > /dev/null)
            execute_process(COMMAND ${SUDO_EXECUTABLE} -E apt upgrade -y > /dev/null)
            execute_process(COMMAND sudo -E apt install -y python3 python3-dev > /dev/null)
        else ()
            execute_process(COMMAND sudo -E apt update)
            execute_process(COMMAND sudo -E apt upgrade -y)
            execute_process(COMMAND sudo -E apt install -y python3 python3-dev)
        endif ()
    endif ()
    console_reset_color()
elseif (DISTRO MATCHES "fedora|rhel|centos")
    console_turn_yellow()
    message(STATUS "Detected Fedora/RHEL-based system")
    console_reset_color()

    console_turn_dim()
    if (${SUPRESS_DEBUG_INFO} STREQUAL "True")
        execute_process(COMMAND sudo -E dnf install -y python3 python3-devel > /dev/null)
    else ()
        execute_process(COMMAND sudo -E dnf install -y python3 python3-devel)
    endif ()
    console_reset_color()
else()
    message(FATAL_ERROR "Unsupported Linux distribution: ${DISTRO}")
endif()

message(STATUS "CMake Module Dependencies Automatic Installation loading completed")
