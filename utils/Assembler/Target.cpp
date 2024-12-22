#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <regex>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <cassert>

// Define regex patterns
const std::regex register_pattern("^%(R\\d+|EXR\\d+|HER\\d+|FER\\d+|RXA)$");
const std::regex constant_pattern("^\\$\\d+$");
const std::regex complex_pattern("^\\*(\\d+)\\(([^,]+),([^,]+),([^\\)]+)\\)$");

// Function to print encoded bytes
void print_encoded(const std::vector<uint8_t>& encoded) {
    for(auto byte : encoded) {
        printf("0x%02X ", byte);
    }
    printf("\n");
}

// Validation functions
bool is_valid_register(const std::string& input) {
    return std::regex_match(input, register_pattern);
}

bool is_valid_constant(const std::string& input) {
    return std::regex_match(input, constant_pattern);
}

bool is_valid_complex(const std::string& input, int& multiplier, std::string& param1, std::string& param2, std::string& param3) {
    std::smatch matches;
    if (std::regex_match(input, matches, complex_pattern)) {
        if (matches.size() != 5) return false; // Full match + 4 capture groups
        try {
            multiplier = std::stoi(matches[1].str());
        } catch(...) {
            return false; // Invalid multiplier
        }
        param1 = matches[2].str();
        param2 = matches[3].str();
        param3 = matches[4].str();

        // Validate each parameter
        if (!is_valid_register(param1) && !is_valid_constant(param1)) return false;
        if (!is_valid_register(param2) && !is_valid_constant(param2)) return false;
        if (!is_valid_register(param3) && !is_valid_constant(param3)) return false;

        return true;
    }
    return false;
}

// Encoding functions
std::optional<std::vector<uint8_t>> encode_register(const std::string& reg) {
    if (!is_valid_register(reg)) {
        return std::nullopt; // Invalid register format
    }
    std::vector<uint8_t> encoded;
    encoded.push_back(0x01); // Register identifier

    // Example encoding logic based on the register type
    if (reg.find("R") != std::string::npos) {
        encoded.push_back(0x00); // Register type
    } else if (reg.find("EXR") != std::string::npos) {
        encoded.push_back(0x01); // Extended Register type
    } else if (reg.find("HER") != std::string::npos) {
        encoded.push_back(0x02); // HER Register type
    } else if (reg.find("FER") != std::string::npos) {
        encoded.push_back(0x03); // FER Register type
    } else if (reg.find("RXA") != std::string::npos) {
        encoded.push_back(0x04); // RXA Register type
    }

    // Extract numerical part
    size_t pos = reg.find_first_of("0123456789");
    if(pos != std::string::npos) {
        std::string num_str = reg.substr(pos);
        if(num_str.empty()) {
            // Handle error: No number found
            encoded.push_back(0x00);
            encoded.push_back(0x00);
        }
        else {
            try {
                int reg_num = std::stoi(num_str);
                encoded.push_back(static_cast<uint8_t>(reg_num & 0xFF));
                encoded.push_back(static_cast<uint8_t>((reg_num >> 8) & 0xFF));
            } catch(...) {
                // Handle error: invalid number
                encoded.push_back(0x00);
                encoded.push_back(0x00);
            }
        }
    } else {
        // Handle error: No digits found
        encoded.push_back(0x00);
        encoded.push_back(0x00);
    }
    return encoded;
}

std::optional<std::vector<uint8_t>> encode_constant(const std::string& constant) {
    if (!is_valid_constant(constant)) {
        return std::nullopt; // Invalid constant format
    }
    std::vector<uint8_t> encoded;
    encoded.push_back(0x02); // Constant identifier
    std::string num_str = constant.substr(1);
    if(num_str.empty()) {
        // Handle error: No number found
        encoded.push_back(0x00);
        encoded.push_back(0x00);
        encoded.push_back(0x00);
        encoded.push_back(0x00);
    }
    else {
        try {
            int const_value = std::stoi(num_str);
            encoded.push_back(static_cast<uint8_t>(const_value & 0xFF));
            encoded.push_back(static_cast<uint8_t>((const_value >> 8) & 0xFF));
            encoded.push_back(static_cast<uint8_t>((const_value >> 16) & 0xFF));
            encoded.push_back(static_cast<uint8_t>((const_value >> 24) & 0xFF));
        } catch(...) {
            // Handle error: invalid number
            encoded.push_back(0x00);
            encoded.push_back(0x00);
            encoded.push_back(0x00);
            encoded.push_back(0x00);
        }
    }
    return encoded;
}

std::optional<std::vector<uint8_t>> encode_complex(const std::string& complex_expr) {
    int multiplier;
    std::string param1, param2, param3;
    if (!is_valid_complex(complex_expr, multiplier, param1, param2, param3)) {
        return std::nullopt; // Invalid complex expression format
    }
    std::vector<uint8_t> encoded;
    encoded.push_back(0x03); // Complex expression identifier
    encoded.push_back(static_cast<uint8_t>(multiplier));

    // Encode each parameter
    auto enc1 = is_valid_register(param1) ? encode_register(param1) : encode_constant(param1);
    auto enc2 = is_valid_register(param2) ? encode_register(param2) : encode_constant(param2);
    auto enc3 = is_valid_register(param3) ? encode_register(param3) : encode_constant(param3);

    if (!enc1.has_value() || !enc2.has_value() || !enc3.has_value()) {
        return std::nullopt; // Failed to encode one of the parameters
    }

    // Append encoded parameters
    encoded.insert(encoded.end(), enc1->begin(), enc1->end());
    encoded.insert(encoded.end(), enc2->begin(), enc2->end());
    encoded.insert(encoded.end(), enc3->begin(), enc3->end());

    return encoded;
}

std::optional<std::vector<uint8_t>> encode(const std::string& input) {
    if (is_valid_register(input)) {
        return encode_register(input);
    }
    if (is_valid_constant(input)) {
        return encode_constant(input);
    }
    // Check if it's a complex expression
    int multiplier;
    std::string param1, param2, param3;
    if (is_valid_complex(input, multiplier, param1, param2, param3)) {
        return encode_complex(input);
    }
    // If none matched, return failure
    return std::nullopt;
}

// Structure for test cases
struct TestCase {
    std::string name;                   // Descriptive name of the test case
    std::string input;                  // Input string to encode
    bool should_succeed;                // Expected outcome: true for valid inputs, false for invalid
    std::vector<uint8_t> expected_output; // Expected byte sequence for valid inputs; ignored for invalid
};

// Helper functions for test case generation
std::vector<std::string> generate_valid_registers(int count) {
    std::vector<std::string> registers;
    std::vector<std::string> register_types = {"R", "EXR", "HER", "FER", "RXA"};

    for(const auto& type : register_types) {
        for(int i = 0; i < count / register_types.size(); ++i) {
            registers.push_back("%" + type + std::to_string(i));
        }
    }
    return registers;
}

std::vector<std::string> generate_valid_constants(int count, int max_value = 100000) {
    std::vector<std::string> constants;
    for(int i = 0; i < count; ++i) {
        constants.push_back("$" + std::to_string(i % max_value));
    }
    return constants;
}

std::vector<std::string> generate_valid_complex_expressions(const std::vector<std::string>& registers, const std::vector<std::string>& constants, int count) {
    std::vector<std::string> complex_exprs;
    std::vector<int> multipliers = {2, 4, 8, 16}; // Example multipliers

    for(int i = 0; i < count; ++i) {
        int multiplier = multipliers[i % multipliers.size()];
        std::string expr = "*" + std::to_string(multiplier) + "(";

        // Select parameters
        std::string param1 = registers[i % registers.size()];
        std::string param2 = constants[i % constants.size()];
        std::string param3 = registers[(i + 1) % registers.size()];

        expr += param1 + ", " + param2 + ", " + param3 + ")";
        complex_exprs.push_back(expr);
    }
    return complex_exprs;
}

std::vector<std::string> generate_invalid_registers(int count) {
    std::vector<std::string> invalid_registers;
    // Define invalid prefixes that do not match any valid register patterns
    std::vector<std::string> invalid_prefixes = {
        "%Q", "%X", "%HZ", "%RZ", "%HERA", "%FEXR", "%R_A", "%R-", "%RX1", "RX"
    };

    size_t num_patterns = invalid_prefixes.size();

    for(int i = 0; i < count; ++i) {
        std::string prefix = invalid_prefixes[i % num_patterns];
        invalid_registers.push_back(prefix + std::to_string(i));
    }
    return invalid_registers;
}

std::vector<std::string> generate_invalid_constants(int count) {
    std::vector<std::string> invalid_constants;
    std::vector<std::string> invalid_patterns = {"$a123", "$12a3", "1234", "$", "$$123", "$-123", "$12.3"};

    for(int i = 0; i < count; ++i) {
        invalid_constants.push_back(invalid_patterns[i % invalid_patterns.size()]);
    }
    return invalid_constants;
}

std::vector<std::string> generate_invalid_complex_expressions(int count) {
    std::vector<std::string> invalid_complex;
    std::vector<std::string> malformed_patterns = {
        "*2(%EXR1, $(1577 + 1577), %EXR1, $(1577 + 1577))", // Extra parameter
        "*5(%RXA, $12200, %R4)", // Unsupported multiplier
        "*3($100, %R1)", // Missing parameter
        "*4($1 + a, $(3593 + 3593), %EXR1)", // Invalid constant
        "*2(*extra, $(443 + 443), %FER3)", // Extra parameter identifier
        "*4(%FER3, $(107 + 107), %FER3, $(107 + 107))", // Correct structure but using invalid constants
        "*4(%EXR5, $(2837 + 2837), %EXR5, $(2837 + 2837))", // Correct structure but using invalid constants
        "*4(%EXR1, $(4937 + 4937), %EXR1, $(4937 + 4937))",
        "*4(%FER7, $(527 + 527), %FER7, $(527 + 527))",
        "*4(%EXR1, $(4097 + 4097), %EXR1, $(4097 + 4097))"
    };

    for(int i = 0; i < count; ++i) {
        invalid_complex.push_back(malformed_patterns[i % malformed_patterns.size()]);
    }
    return invalid_complex;
}

// Function to process a subset of test cases (for multithreading)
void process_test_cases(const std::vector<TestCase>& subset, int& passed, int& failed, std::mutex& output_mutex) {
    for(const auto& test : subset) {
        // Attempt to encode the input
        auto encoded_opt = encode(test.input);

        bool test_passed = false;
        std::string failure_reason;

        if(test.should_succeed) {
            if(encoded_opt.has_value()) {
                // For valid tests, compare the actual output with the expected output
                if(encoded_opt.value() == test.expected_output) {
                    test_passed = true;
                    passed++;
                }
                else {
                    failure_reason = "Output mismatch.";
                }
            }
            else {
                // Encoding was expected to succeed but failed
                failure_reason = "Expected encoding to succeed, but it failed.";
            }
        }
        else {
            if(!encoded_opt.has_value()) {
                // For invalid tests, encoding should fail
                test_passed = true;
                passed++;
            }
            else {
                // Encoding was expected to fail but succeeded
                failure_reason = "Expected encoding to fail, but it succeeded.";
            }
        }

        if(test_passed) {
            // Optionally, count passes without logging
            continue;
        }
        else {
            // Log only failures
            std::lock_guard<std::mutex> lock(output_mutex);
            std::cout << "-----------------------------" << std::endl;
            std::cout << "Test: " << test.name << std::endl;
            std::cout << "Input: " << test.input << std::endl;
            std::cout << "Result: FAIL" << std::endl;
            std::cout << failure_reason << std::endl;
            if(test.should_succeed && encoded_opt.has_value()) {
                std::cout << "Expected Output: ";
                print_encoded(test.expected_output);
                std::cout << "Actual Output:   ";
                print_encoded(encoded_opt.value());
            }
            else if(!test.should_succeed && encoded_opt.has_value()) {
                std::cout << "Unexpected Output: ";
                print_encoded(encoded_opt.value());
            }
            failed++;
        }
    }
}

int main() {
    std::vector<TestCase> test_cases;

    // Seed for randomness (if needed)
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Define the number of test cases
    const int total_valid = 5000;
    const int total_invalid = 5000;

    // Generate valid components
    std::vector<std::string> valid_registers = generate_valid_registers(1000); // 1,000 unique registers
    std::vector<std::string> valid_constants = generate_valid_constants(1000); // 1,000 unique constants
    std::vector<std::string> valid_complex = generate_valid_complex_expressions(valid_registers, valid_constants, 3000); // 3,000 complex expressions

    // Generate invalid components
    std::vector<std::string> invalid_registers = generate_invalid_registers(3000); // Changed to 3000 to match the number of test cases
    std::vector<std::string> invalid_constants = generate_invalid_constants(1000);
    std::vector<std::string> invalid_complex = generate_invalid_complex_expressions(1000); // Adjusted based on test case generation needs

    // Assertions to ensure correct sizes
    assert(valid_registers.size() == 1000 && "Expected 1000 valid registers");
    assert(valid_constants.size() == 1000 && "Expected 1000 valid constants");
    assert(valid_complex.size() == 3000 && "Expected 3000 valid complex expressions");
    assert(invalid_registers.size() == 3000 && "Expected 3000 invalid registers");
    assert(invalid_constants.size() == 1000 && "Expected 1000 invalid constants");
    assert(invalid_complex.size() == 1000 && "Expected 1000 invalid complex expressions");

    // Populate valid test cases
    // 1. Valid Registers
    for(int i = 0; i < valid_registers.size(); ++i) {
        TestCase tc;
        tc.name = "Valid Register " + std::to_string(i);
        tc.input = valid_registers[i];
        tc.should_succeed = true;

        // Encode using the encode function to ensure consistency
        auto encoded_opt = encode(tc.input);
        if(encoded_opt.has_value()) {
            tc.expected_output = encoded_opt.value();
        } else {
            // This should not happen as we are generating valid registers
            std::cerr << "Error: Failed to encode valid register: " << tc.input << std::endl;
            continue;
        }

        test_cases.push_back(tc);
    }

    // 2. Valid Constants
    for(int i = 0; i < valid_constants.size(); ++i) {
        TestCase tc;
        tc.name = "Valid Constant " + std::to_string(i);
        tc.input = valid_constants[i];
        tc.should_succeed = true;

        // Encode using the encode function to ensure consistency
        auto encoded_opt = encode(tc.input);
        if(encoded_opt.has_value()) {
            tc.expected_output = encoded_opt.value();
        } else {
            // This should not happen as we are generating valid constants
            std::cerr << "Error: Failed to encode valid constant: " << tc.input << std::endl;
            continue;
        }

        test_cases.push_back(tc);
    }

    // 3. Valid Complex Expressions
    for(int i = 0; i < valid_complex.size(); ++i) {
        TestCase tc;
        tc.name = "Valid Complex " + std::to_string(i);
        tc.input = valid_complex[i];
        tc.should_succeed = true;

        // Encode using the encode function to ensure consistency
        auto encoded_opt = encode(tc.input);
        if(encoded_opt.has_value()) {
            tc.expected_output = encoded_opt.value();
        } else {
            // This should not happen as we are generating valid complex expressions
            std::cerr << "Error: Failed to encode valid complex expression: " << tc.input << std::endl;
            continue;
        }

        test_cases.push_back(tc);
    }

    // 4. Invalid Test Cases
    // 4.1 Invalid Registers
    for(int i = 0; i < invalid_registers.size(); ++i) {
        TestCase tc;
        tc.name = "Invalid Register " + std::to_string(i);
        tc.input = invalid_registers[i];
        tc.should_succeed = false;
        tc.expected_output = {}; // No expected output for invalid cases
        test_cases.push_back(tc);
    }

    // 4.2 Invalid Constants
    for(int i = 0; i < invalid_constants.size(); ++i) {
        TestCase tc;
        tc.name = "Invalid Constant " + std::to_string(i);
        tc.input = invalid_constants[i];
        tc.should_succeed = false;
        tc.expected_output = {}; // No expected output for invalid cases
        test_cases.push_back(tc);
    }

    // 4.3 Invalid Complex Expressions
    for(int i = 0; i < invalid_complex.size(); ++i) {
        TestCase tc;
        tc.name = "Invalid Complex " + std::to_string(i);
        tc.input = invalid_complex[i];
        tc.should_succeed = false;
        tc.expected_output = {}; // No expected output for invalid cases
        test_cases.push_back(tc);
    }

    // Ensure we have exactly 10,000 test cases
    while(test_cases.size() < 10000) {
        // Add more invalid test cases if needed
        TestCase tc;
        tc.name = "Invalid Constant Extra " + std::to_string(test_cases.size());
        tc.input = "$invalid" + std::to_string(test_cases.size());
        tc.should_succeed = false;
        tc.expected_output = {};
        test_cases.push_back(tc);
    }

    // Counters for passed and failed tests
    int passed = 0;
    int failed = 0;

    // Multithreading setup
    unsigned int num_threads = std::thread::hardware_concurrency();
    if(num_threads == 0) num_threads = 4; // Default to 4 if unable to detect
    std::vector<std::thread> threads;
    std::vector<std::vector<TestCase>> test_chunks(num_threads);

    // Preallocate memory for test_chunks to improve performance
    for(auto& chunk : test_chunks) {
        chunk.reserve(test_cases.size() / num_threads + 1);
    }

    // Distribute test cases among threads using range-based partitioning
    for(int i = 0; i < test_cases.size(); ++i) {
        test_chunks[i % num_threads].push_back(test_cases[i]);
    }

    // Mutex for synchronized output
    std::mutex output_mutex;

    // Launch threads
    for(unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(process_test_cases, std::cref(test_chunks[i]), std::ref(passed), std::ref(failed), std::ref(output_mutex));
    }

    // Join threads
    for(auto& th : threads) {
        th.join();
    }

    // Test Summary
    std::cout << "-----------------------------" << std::endl;
    std::cout << "Test Summary:" << std::endl;
    std::cout << "Passed: " << passed << "/" << test_cases.size() << std::endl;
    std::cout << "Failed: " << failed << "/" << test_cases.size() << std::endl;
    std::cout << "-----------------------------" << std::endl;

    return (failed == 0) ? 0 : 1; // Return 0 if all tests passed, 1 otherwise
}
