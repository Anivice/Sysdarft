message(STATUS "CMake Module Sysdarft Test Case Loaded")

function(sysdarft_add_test TEST_NAME TEST_FILES)
    add_executable("Sysdarft_test_case_${TEST_NAME}"  ${TEST_FILES})
    add_test(NAME "Sysdarft_test_case_${TEST_NAME}_" COMMAND "Sysdarft_test_case_${TEST_NAME}")
    message(STATUS "Sysdarft Test Case `${TEST_NAME}` Enabled")
endfunction()
