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

set(RESOURCE_FILE_LIST_H "${CMAKE_BINARY_DIR}/include/resource_file_list.h")
set(RESOURCE_FILE_DIR    "${CMAKE_BINARY_DIR}/res")
set(BUILD_LOCK           "${CMAKE_BINARY_DIR}/xxd.lock")
set(PROJECT_ROOT_DIR     "${CMAKE_SOURCE_DIR}")
set(PROJECT_RESOURCE_FILE_LIST_FILE "${CMAKE_SOURCE_DIR}/resources.txt")

execute_process(COMMAND ${MKDIR_EXECUTABLE} -p "${RESOURCE_FILE_DIR}")
execute_process(COMMAND ${MKDIR_EXECUTABLE} -p "${CMAKE_BINARY_DIR}/include")
execute_process(COMMAND ${TOUCH_EXECUTABLE} "${RESOURCE_FILE_LIST_H}")

include_directories("${CMAKE_BINARY_DIR}/include")

add_custom_target(xxd_compile_target ALL
        COMMAND ${BASH_EXECUTABLE} -c
        "${CMAKE_SOURCE_DIR}/scripts/compile_resource_files.bash \
                    \"${RESOURCE_FILE_LIST_H}\"             \
                    \"${RESOURCE_FILE_DIR}\"                \
                    \"${PROJECT_ROOT_DIR}\"                 \
                    \"${PROJECT_RESOURCE_FILE_LIST_FILE}\"  \
                    \"${BUILD_LOCK}\""
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating resource header file..."
        VERBATIM
)

# Function to add a target for xxd conversion and update resource file list
function(sysdarft_add_xxd_target TARGET_NAME TARGET_FILE HOST_NAME)
    set(TARGET_NAME "xxd_${TARGET_NAME}")

    string(REPLACE "/" "_" OUTPUT_FILENAME "${TARGET_FILE}")
    execute_process(COMMAND ${TOUCH_EXECUTABLE} "${CMAKE_BINARY_DIR}/res/xxd_${OUTPUT_FILENAME}.cpp")

    add_library(${TARGET_NAME}_binary_content STATIC
            "${CMAKE_BINARY_DIR}/res/xxd_${OUTPUT_FILENAME}.cpp"
            "${RESOURCE_FILE_LIST_H}"
    )
    add_dependencies(${TARGET_NAME}_binary_content xxd_compile_target) # target that will generate C code
    target_link_libraries(${HOST_NAME} PUBLIC ${TARGET_NAME}_binary_content)
endfunction()

message(STATUS "CMake Module XXD Target Compiler loading complete")
