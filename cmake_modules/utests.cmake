set(MODULE_NAME "UnitTests")
set(LOADED_MODULE_LIST "${LOADED_MODULE_LIST}")
foreach (Module IN LISTS LOADED_MODULE_LIST)
    if ("X${Module}" STREQUAL "X${MODULE_NAME}")
        return()
    endif ()
endforeach (Module IN LISTS LOADED_MODULE_LIST)
list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")

check_program(bash BASH)

message(STATUS "CMake Module Sysdarft Test Case Loaded")
enable_testing()

function(sysdarft_add_test TEST_NAME TEST_FILES)
    add_executable("Sysdarft_test_case_${TEST_NAME}" ${TEST_FILES})
    add_test(NAME "Sysdarft_test_case_${TEST_NAME}_exe"
            COMMAND ${BASH_EXECUTABLE} -c "ASAN_OPTIONS=detect_leaks=0 ${CMAKE_BINARY_DIR}/Sysdarft_test_case_${TEST_NAME}")

    console_turn_green()
    message(STATUS "Sysdarft Test Case `${TEST_NAME}` Enabled")
    console_reset_color()
endfunction()

message(STATUS "CMake Module Sysdarft Test Case loading complete")
