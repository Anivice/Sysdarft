set(MODULE_NAME "xxdObjectCompiler")
set(LOADED_MODULE_LIST "${LOADED_MODULE_LIST}")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    if ("X${Module}" STREQUAL "X${MODULE_NAME}")
        return()
    endif ()
endforeach (Module IN LISTS LOADED_MODULE_LIST)
list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")

message(STATUS "CMake Module XXD Target Compiler Loaded")

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")

check_program("bash"    "BASH")
check_program("touch"   "TOUCH")
check_program("mkdir"   "MKDIR")

set(RESOURCE_FILE_LIST_H    "${CMAKE_BINARY_DIR}/include/resource_file_list.h")
set(RESOURCE_FILE_LIST_CPP  "${CMAKE_BINARY_DIR}/res/resource_file_list.cpp")
set(RESOURCE_FILE_DIR       "${CMAKE_BINARY_DIR}/res")
set(PROJECT_ROOT_DIR        "${CMAKE_SOURCE_DIR}")
set(BUILD_LOCK              "${CMAKE_BINARY_DIR}/xxd.lock")
set(PROJECT_RESOURCE_FILE_LIST_FILE "${CMAKE_BINARY_DIR}/resources.txt")

execute_process(COMMAND ${MKDIR_EXECUTABLE} -p "${RESOURCE_FILE_DIR}")
execute_process(COMMAND ${MKDIR_EXECUTABLE} -p "${CMAKE_BINARY_DIR}/include")
execute_process(COMMAND ${TOUCH_EXECUTABLE} "${RESOURCE_FILE_LIST_H}")
execute_process(COMMAND ${TOUCH_EXECUTABLE} "${RESOURCE_FILE_LIST_CPP}")

execute_process(COMMAND ${BASH_EXECUTABLE} -c "
        IFS=$'\n';
        cd \"\$\( cd -- \"$( dirname -- \"\${BASH_SOURCE[0]}\" \)\" &> /dev/null && pwd \)\" || exit 1;
        cd .. || exit 1;
        LIST=$(git ls-files --others --exclude-standard --cached && git diff --name-only --diff-filter=ADR);
        {
            for FILE in $LIST; do
                echo $FILE;
            done;
        } > ${PROJECT_RESOURCE_FILE_LIST_FILE}")

include_directories("${CMAKE_BINARY_DIR}/include")

add_custom_target(xxd_compile_target ALL
        COMMAND ${BASH_EXECUTABLE} -c
        "${CMAKE_SOURCE_DIR}/scripts/compile_resource_files.bash \
                    \"${RESOURCE_FILE_LIST_H}\"             \
                    \"${RESOURCE_FILE_LIST_CPP}\"           \
                    \"${RESOURCE_FILE_DIR}\"                \
                    \"${PROJECT_ROOT_DIR}\"                 \
                    \"${PROJECT_RESOURCE_FILE_LIST_FILE}\"  \
                    \"${BUILD_LOCK}\""
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating resource header file..."
        VERBATIM
)

add_library(xxd_binary_content STATIC "${RESOURCE_FILE_LIST_H}" "${RESOURCE_FILE_LIST_CPP}")
add_dependencies(xxd_binary_content xxd_compile_target) # target that will generate C code

# Function to add a target for xxd conversion and update resource file list
function(sysdarft_xxd_link_library HOST_NAME)
    target_link_libraries(${HOST_NAME} PUBLIC xxd_binary_content)
endfunction()

message(STATUS "CMake Module XXD Target Compiler loading complete")
