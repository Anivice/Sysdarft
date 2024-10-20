set(MODULE_NAME "xxdObjectCompiler")
if (NOT "${MODULE_NAME}" IN_LIST LOADED_MODULE_LIST)
    list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")
else()
    return()
endif()

message(STATUS "CMake Module XXD Target Compiler Loaded")

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")

# Check for required programs
check_program("bash" "BASH")
check_program("touch" "TOUCH")
check_program("mkdir" "MKDIR")

# Set paths
set(RESOURCE_FILE_LIST_H        "${CMAKE_BINARY_DIR}/include/resource_file_list.h")
set(RESOURCE_FILE_LIST_CPP      "${CMAKE_BINARY_DIR}/res/resource_file_list.cpp")
set(RESOURCE_FILE_DIR           "${CMAKE_BINARY_DIR}/res")
set(PROJECT_RESOURCE_FILE_LIST  "${CMAKE_BINARY_DIR}/resources.txt")
set(BUILD_LOCK                  "${CMAKE_BINARY_DIR}/xxd.lock")

# Create required directories and files
file(MAKE_DIRECTORY "${RESOURCE_FILE_DIR}" "${CMAKE_BINARY_DIR}/include")
file(TOUCH "${RESOURCE_FILE_LIST_H}" "${RESOURCE_FILE_LIST_CPP}")

# Generate resource file list using git
execute_process(
        COMMAND ${BASH_EXECUTABLE} -c "
        cd \"${CMAKE_SOURCE_DIR}\" || exit 1;
        git ls-files --others --exclude-standard --cached > ${PROJECT_RESOURCE_FILE_LIST};
        git diff --name-only --diff-filter=ADR >> ${PROJECT_RESOURCE_FILE_LIST}"
)

# Include directories
include_directories("${CMAKE_BINARY_DIR}/include")

# Custom target to compile resource files
add_custom_target(xxd_compile_target ALL
        COMMAND ${BASH_EXECUTABLE} -c
            "${CMAKE_SOURCE_DIR}/scripts/compile_resource_files.bash \
            \"${RESOURCE_FILE_LIST_H}\"             \
            \"${RESOURCE_FILE_LIST_CPP}\"           \
            \"${RESOURCE_FILE_DIR}\"                \
            \"${CMAKE_SOURCE_DIR}\"                 \
            \"${PROJECT_RESOURCE_FILE_LIST}\"       \
            \"${BUILD_LOCK}\""
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating resource header file..."
        VERBATIM
)

# Create shared library for xxd binary content
add_library(xxd_binary_content SHARED "${RESOURCE_FILE_LIST_H}" "${RESOURCE_FILE_LIST_CPP}")
add_dependencies(xxd_binary_content xxd_compile_target)
target_compile_options(xxd_binary_content PUBLIC -fPIC)

# Function to link library with xxd binary content
function(sysdarft_xxd_link_library HOST_NAME)
    target_link_libraries(${HOST_NAME} PUBLIC xxd_binary_content)
endfunction()

message(STATUS "CMake Module XXD Target Compiler loading complete")
