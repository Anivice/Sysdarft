set(MODULE_NAME "UnitTests")
if (NOT "${MODULE_NAME}" IN_LIST LOADED_MODULE_LIST)
    list(APPEND LOADED_MODULE_LIST "${MODULE_NAME}")
else()
    return()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/check_program.cmake")
check_program(bash BASH)

message(STATUS "CMake Module Sysdarft Test Case Loaded")
enable_testing()

# Function to add test cases
function(sysdarft_add_test TEST_NAME TEST_FILES LIBRARIES)
    set(TEST_EXECUTABLE "Sysdarft_test_case_${TEST_NAME}")
    add_executable(${TEST_EXECUTABLE} ${TEST_FILES})
    target_link_libraries(${TEST_EXECUTABLE} PUBLIC ${LIBRARIES})

    add_test(NAME "${TEST_EXECUTABLE}_exe"
            COMMAND ${BASH_EXECUTABLE} -c "ASAN_OPTIONS=detect_leaks=0 ${CMAKE_BINARY_DIR}/${TEST_EXECUTABLE}")

    console_turn_green()
    message(STATUS "Sysdarft Test Case `${TEST_NAME}` Enabled")
    console_reset_color()
endfunction()

message(STATUS "CMake Module Sysdarft Test Case loading complete")
