set(MODULE_NAME "DependenciesAutoInstall")
if (NOT "${MODULE_NAME}" IN_LIST LOADED_MODULE_LIST)
    list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")
    message(STATUS "CMake Module Dependencies Automatic Installation Loaded")
else()
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/color_the_console.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")

# Check for required programs
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

# Define packages for different distributions
set(debian_packages python3 python3-dev python3-venv fuse3 libfuse-dev vim)
set(fedora_packages python3 python3-devel python3-virtualenv fuse3 fuse3-devel fuse fuse-devel vim)

# Function to execute package installation
function(install_packages package_manager packages)
    if (NOT "${DISABLE_APT_UPDATE}" STREQUAL "True")
        if (${SUPRESS_DEBUG_INFO} STREQUAL "True")
            execute_process(COMMAND ${SUDO_EXECUTABLE} -E ${package_manager} install -y ${packages} > /dev/null)
        else()
            console_turn_dim()
            execute_process(COMMAND ${SUDO_EXECUTABLE} -E ${package_manager} install -y ${packages})
            console_reset_color()
        endif()
    endif()
endfunction()

# Detect and handle the Linux distribution
if (DISTRO MATCHES "ubuntu|debian")
    console_turn_yellow()
    message(STATUS "Detected Ubuntu/Debian-based system")
    console_reset_color()

    install_packages("apt" "${debian_packages}")

elseif (DISTRO MATCHES "fedora|rhel|centos")
    console_turn_yellow()
    message(STATUS "Detected Fedora/RHEL-based system")
    console_reset_color()

    install_packages("dnf" "${fedora_packages}")

else()
    message(FATAL_ERROR "Unsupported Linux distribution: ${DISTRO}")
endif()

message(STATUS "CMake Module Dependencies Automatic Installation loading completed")
