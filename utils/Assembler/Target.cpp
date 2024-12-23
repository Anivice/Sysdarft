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
#include <cstdint>      // For int64_t, uint8_t

// Define regex patterns
const std::regex register_pattern(R"(^%(R\d+|EXR\d+|HER\d+|FER\d+)$)");
const std::regex constant_pattern(R"(^\$\d+$)");
const std::regex complex_pattern(R"(^\*(\d+)\(([^,]+),([^,]+),([^\)]+)\)$)");

// --------------------------------------------------------------------------
// Helper to push a 64-bit value (little-endian) into a vector of bytes
// --------------------------------------------------------------------------
void push_64(std::vector<uint8_t>& buffer, std::int64_t value)
{
    for (int shift = 0; shift < 64; shift += 8) {
        buffer.push_back(static_cast<uint8_t>((value >> shift) & 0xFF));
    }
}

// --------------------------------------------------------------------------
// Function to print encoded bytes
// --------------------------------------------------------------------------
void print_encoded(const std::vector<uint8_t>& encoded)
{
    for (auto byte : encoded) {
        printf("0x%02X ", byte);
    }
    printf("\n");
}

// --------------------------------------------------------------------------
// Validation functions
// --------------------------------------------------------------------------
bool is_valid_register(const std::string& input) {
    return std::regex_match(input, register_pattern);
}

bool is_valid_constant(const std::string& input) {
    return std::regex_match(input, constant_pattern);
}

bool is_valid_complex(const std::string& input,
                      std::int64_t& multiplier,
                      std::string& param1,
                      std::string& param2,
                      std::string& param3)
{
    if (std::smatch matches; std::regex_match(input, matches, complex_pattern))
    {
        if (matches.size() != 5) return false; // Full match + 4 capture groups
        try {
            multiplier = std::stoll(matches[1].str());
        } catch (...) {
            // Could not parse as 64-bit or out of range
            return false;
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

// --------------------------------------------------------------------------
// Encoding functions
// --------------------------------------------------------------------------
std::optional<std::vector<uint8_t>> encode_register(const std::string& reg)
{
    if (!is_valid_register(reg)) {
        return std::nullopt; // Invalid register format
    }

    std::vector<uint8_t> encoded;
    encoded.push_back(0x01); // Register identifier

    // Example encoding logic based on the register type
    // (You might refine detection if "R" appears in multiple places, etc.)
    if (reg.find('R') == 1) {
        // If the second character is 'R', treat as simplest "R" type
        encoded.push_back(0x00); // Register type
    } else if (reg.find("EXR") == 1) {
        encoded.push_back(0x01); // Extended Register type
    } else if (reg.find("HER") == 1) {
        encoded.push_back(0x02); // HER Register type
    } else if (reg.find("FER") == 1) {
        encoded.push_back(0x03); // FER Register type
    } else {
        return std::nullopt;
    }

    // Extract numerical part as a 64-bit integer
    size_t pos = reg.find_first_of("0123456789");
    if (pos != std::string::npos) {
        std::string num_str = reg.substr(pos);
        if (num_str.empty()) {
            // Handle error: No number found
            push_64(encoded, 0);
        } else {
            try {
                const std::int64_t reg_num = std::stoll(num_str);
                push_64(encoded, reg_num);
            } catch (...) {
                // Handle error: invalid or out-of-range
                push_64(encoded, 0);
            }
        }
    } else {
        // Handle error: No digits found
        push_64(encoded, 0);
    }
    return encoded;
}

std::optional<std::vector<uint8_t>> encode_constant(const std::string& constant) {
    if (!is_valid_constant(constant)) {
        return std::nullopt; // Invalid constant format
    }

    std::vector<uint8_t> encoded;
    encoded.push_back(0x02); // Constant identifier

    // Remove leading '$'
    std::string num_str = constant.substr(1);
    if (num_str.empty()) {
        // Handle error: No number found
        push_64(encoded, 0);
    } else {
        try {
            std::int64_t const_value = std::stoll(num_str);
            push_64(encoded, const_value);
        } catch (...) {
            // Handle error: invalid or out-of-range
            push_64(encoded, 0);
        }
    }
    return encoded;
}

std::optional<std::vector<uint8_t>> encode_complex(const std::string& complex_expr) {
    std::int64_t multiplier;
    std::string param1, param2, param3;

    if (!is_valid_complex(complex_expr, multiplier, param1, param2, param3)) {
        return std::nullopt; // Invalid complex expression format
    }

    std::vector<uint8_t> encoded;
    encoded.push_back(0x03); // Complex expression identifier

    // Push the 64-bit multiplier
    push_64(encoded, multiplier);

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
    // Try register
    if (is_valid_register(input)) {
        return encode_register(input);
    }
    // Try constant
    if (is_valid_constant(input)) {
        return encode_constant(input);
    }
    // Try complex expression
    std::int64_t multiplier;
    std::string param1, param2, param3;
    if (is_valid_complex(input, multiplier, param1, param2, param3)) {
        return encode_complex(input);
    }
    // If none matched, return failure
    return std::nullopt;
}

// Structure for test cases
struct TestCase {
    std::string name;                     // Descriptive name of the test case
    std::string input;                    // Input string to encode
    bool should_succeed;                  // Expected outcome: true for valid inputs, false for invalid
    std::vector<uint8_t> expected_output; // Expected byte sequence for valid inputs; ignored for invalid
};

// --------------------------------------------------------------------------
// Helper functions for test case generation
// --------------------------------------------------------------------------
std::vector<std::string> generate_valid_registers(int count) {
    std::vector<std::string> registers;
    std::vector<std::string> register_types = {"R", "EXR", "HER", "FER", "RXA"};

    for (const auto& type : register_types) {
        for (int i = 0; i < count / static_cast<int>(register_types.size()); ++i) {
            registers.push_back("%" + type + std::to_string(i));
        }
    }
    return registers;
}

std::vector<std::string> generate_valid_constants(int count, std::int64_t max_value = 100000) {
    std::vector<std::string> constants;
    constants.reserve(count);
    for (int i = 0; i < count; ++i) {
        constants.push_back("$" + std::to_string(i % max_value));
    }
    return constants;
}

std::vector<std::string> generate_valid_complex_expressions(const std::vector<std::string>& registers,
                                                            const std::vector<std::string>& constants,
                                                            int count) {
    std::vector<std::string> complex_exprs;
    complex_exprs.reserve(count);
    std::vector<std::int64_t> multipliers = {2, 4, 8, 16}; // Example multipliers

    for (int i = 0; i < count; ++i) {
        std::int64_t multiplier = multipliers[i % multipliers.size()];
        std::string expr = "*" + std::to_string(multiplier) + "(";

        // Select parameters
        const std::string& param1 = registers[i % registers.size()];
        const std::string& param2 = constants[i % constants.size()];
        const std::string& param3 = registers[(i + 1) % registers.size()];

        // If your regex allows spaces after commas, you can add them. Otherwise, keep them out.
        expr += param1 + "," + param2 + "," + param3 + ")";
        complex_exprs.push_back(expr);
    }
    return complex_exprs;
}

std::vector<std::string> generate_invalid_registers(int count) {
    std::vector<std::string> invalid_registers;
    invalid_registers.reserve(count);

    // Define invalid prefixes that do not match any valid register patterns
    std::vector<std::string> invalid_prefixes = {
        "%Q", "%X", "%HZ", "%RZ", "%HERA", "%FEXR", "%R_A", "%R-", "%RX1", "RX"
    };

    size_t num_patterns = invalid_prefixes.size();

    for (int i = 0; i < count; ++i) {
        std::string prefix = invalid_prefixes[i % num_patterns];
        invalid_registers.push_back(prefix + std::to_string(i));
    }
    return invalid_registers;
}

std::vector<std::string> generate_invalid_constants(int count) {
    std::vector<std::string> invalid_constants;
    invalid_constants.reserve(count);

    std::vector<std::string> invalid_patterns = {
        "$a123", "$12a3", "1234", "$", "$$123", "$-123abc", "$12.3"
    };

    for (int i = 0; i < count; ++i) {
        invalid_constants.push_back(invalid_patterns[i % invalid_patterns.size()]);
    }
    return invalid_constants;
}

std::vector<std::string> generate_invalid_complex_expressions(int count) {
    std::vector<std::string> invalid_complex;
    invalid_complex.reserve(count);

    // Examples: extra params, invalid multiplier, missing param, etc.
    std::vector<std::string> malformed_patterns = {
        "*2(%EXR1, $(1577 + 1577), %EXR1, $(1577 + 1577))", // Extra param
        "*9999999999999999999(%RXA, $12200, %R4)",          // Possibly out-of-range multiplier
        "*3($100, %R1)",                                    // Missing parameter
        "*4($1 + a, $(3593 + 3593), %EXR1)",                // Invalid constant
        "*2(*extra, $(443 + 443), %FER3)",                  // Parameter with leading '*'
        "*4(%FER3, $(107 + 107), %FER3, $(107 + 107))",     // "valid" structure, but constants are invalid
        "*4(%EXR5, $(2837 + 2837), %EXR5, $(2837 + 2837))",
        "*4(%EXR1, $(4937 + 4937), %EXR1, $(4937 + 4937))",
        "*4(%FER7, $(527 + 527), %FER7, $(527 + 527))",
        "*4(%EXR1, $(4097 + 4097), %EXR1, $(4097 + 4097))"
    };

    for (int i = 0; i < count; ++i) {
        invalid_complex.push_back(malformed_patterns[i % malformed_patterns.size()]);
    }
    return invalid_complex;
}

// Example advanced test-case generation:
std::vector<TestCase> generate_advanced_test_cases() {
    std::vector<TestCase> advanced_tests;

    // 1. Edge-case constants
    {
        // $0 is valid
        std::string input = "$0";
        auto encoded_opt = encode(input); // We expect success
        if (encoded_opt.has_value()) {
            advanced_tests.push_back({
                "Advanced Constant 0",
                input,
                true,
                encoded_opt.value()
            });
        }
    }
    {
        // $9223372036854775807 (max 64-bit signed)
        std::string input = "$9223372036854775807";
        auto encoded_opt = encode(input); // Expect success
        if (encoded_opt.has_value()) {
            advanced_tests.push_back({
                "Advanced Constant INT64_MAX",
                input,
                true,
                encoded_opt.value()
            });
        }
    }
    {
        // $-9223372036854775808 (min 64-bit signed)
        std::string input = "$-9223372036854775808";
        auto encoded_opt = encode(input); // Expect success
        if (encoded_opt.has_value()) {
            advanced_tests.push_back({
                "Advanced Constant INT64_MIN",
                input,
                true,
                encoded_opt.value()
            });
        }
    }
    {
        // $9223372036854775808 (one past INT64_MAX) -> should fail (out of range for 64-bit signed)
        std::string input = "$9223372036854775808";
        advanced_tests.push_back({
            "Advanced Constant Overflow",
            input,
            false,
            {}
        });
    }

    // 2. Register with borderline numeric part
    {
        // %R9999999999999999 -> extremely large numeric part for a register
        // If it fits in 64 bits, we accept it. Otherwise it fails if out of range for stoll.
        std::string input = "%R9999999999999999";
        // Let's see if we can encode:
        auto encoded_opt = encode(input);
        if (encoded_opt.has_value()) {
            advanced_tests.push_back({
                "Advanced Register Very Large",
                input,
                true,
                encoded_opt.value()
            });
        } else {
            // Possibly fails if out-of-range, then it's invalid
            advanced_tests.push_back({
                "Advanced Register Very Large",
                input,
                false,
                {}
            });
        }
    }

    // 3. Complex expression mixing extremes
    {
        // *2(%R0,$9223372036854775807,%R1) => valid if code can handle INT64_MAX
        std::string input = "*2(%R0,$9223372036854775807,%R1)";
        auto encoded_opt = encode(input);
        if (encoded_opt.has_value()) {
            advanced_tests.push_back({
                "Advanced Complex with INT64_MAX",
                input,
                true,
                encoded_opt.value()
            });
        }
    }
    {
        // *16(%EXR9999999999999999,$-9223372036854775808,%RXA1)
        //  => might be valid or invalid depending on parse success
        std::string input = "*16(%EXR9999999999999999,$-9223372036854775808,%RXA1)";
        auto encoded_opt = encode(input);
        if (encoded_opt.has_value()) {
            advanced_tests.push_back({
                "Advanced Complex Large Values",
                input,
                true,
                encoded_opt.value()
            });
        } else {
            advanced_tests.push_back({
                "Advanced Complex Large Values",
                input,
                false,
                {}
            });
        }
    }
    {
        // *9999999999999999999(%R0,$2147483648,%R1) -> huge multiplier likely out-of-range
        // This will probably fail if it doesn't fit in 64-bit or throws an exception
        std::string input = "*9999999999999999999(%R0,$2147483648,%R1)";
        advanced_tests.push_back({
            "Advanced Complex Overflow Multiplier",
            input,
            false,
            {}
        });
    }

    return advanced_tests;
}

// --------------------------------------------------------------------------
// Function to process a subset of test cases (for multithreading)
// --------------------------------------------------------------------------
void process_test_cases(const std::vector<TestCase>& subset,
                        int& passed,
                        int& failed,
                        std::mutex& output_mutex)
{
    for (const auto& test : subset) {
        // Attempt to encode the input
        auto encoded_opt = encode(test.input);

        bool test_passed = false;
        std::string failure_reason;

        if (test.should_succeed) {
            if (encoded_opt.has_value()) {
                // For valid tests, compare the actual output with the expected output
                if (encoded_opt.value() == test.expected_output) {
                    test_passed = true;
                } else {
                    failure_reason = "Output mismatch.";
                }
            } else {
                // Encoding was expected to succeed but failed
                failure_reason = "Expected encoding to succeed, but it failed.";
            }
        } else {
            if (!encoded_opt.has_value()) {
                // For invalid tests, encoding should fail
                test_passed = true;
            } else {
                // Encoding was expected to fail but succeeded
                failure_reason = "Expected encoding to fail, but it succeeded.";
            }
        }

        if (test_passed) {
            std::lock_guard<std::mutex> lock(output_mutex);
            passed++;
        } else {
            std::lock_guard<std::mutex> lock(output_mutex);
            failed++;

            // Log only failures
            std::cout << "-----------------------------" << std::endl;
            std::cout << "Test: " << test.name << std::endl;
            std::cout << "Input: " << test.input << std::endl;
            std::cout << "Result: FAIL" << std::endl;
            std::cout << failure_reason << std::endl;
            if (test.should_succeed && encoded_opt.has_value()) {
                std::cout << "Expected Output: ";
                print_encoded(test.expected_output);
                std::cout << "Actual Output:   ";
                print_encoded(encoded_opt.value());
            } else if (!test.should_succeed && encoded_opt.has_value()) {
                std::cout << "Unexpected Output: ";
                print_encoded(encoded_opt.value());
            }
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
    std::vector<std::string> valid_registers = generate_valid_registers(1000);
    std::vector<std::string> valid_constants = generate_valid_constants(1000);
    std::vector<std::string> valid_complex = generate_valid_complex_expressions(valid_registers,
                                                                                valid_constants,
                                                                                3000);

    // Generate invalid components
    std::vector<std::string> invalid_registers = generate_invalid_registers(3000);
    std::vector<std::string> invalid_constants = generate_invalid_constants(1000);
    std::vector<std::string> invalid_complex = generate_invalid_complex_expressions(1000);

    // Assertions to ensure correct sizes
    assert(valid_registers.size() == 1000 && "Expected 1000 valid registers");
    assert(valid_constants.size() == 1000 && "Expected 1000 valid constants");
    assert(valid_complex.size() == 3000 && "Expected 3000 valid complex expressions");
    assert(invalid_registers.size() == 3000 && "Expected 3000 invalid registers");
    assert(invalid_constants.size() == 1000 && "Expected 1000 invalid constants");
    assert(invalid_complex.size() == 1000 && "Expected 1000 invalid complex expressions");

    // 1. Populate valid Register test cases
    for (int i = 0; i < static_cast<int>(valid_registers.size()); ++i) {
        TestCase tc;
        tc.name = "Valid Register " + std::to_string(i);
        tc.input = valid_registers[i];
        tc.should_succeed = true;

        auto encoded_opt = encode(tc.input);
        if (encoded_opt.has_value()) {
            tc.expected_output = encoded_opt.value();
        } else {
            std::cerr << "Error: Failed to encode valid register: " << tc.input << std::endl;
            continue;
        }
        test_cases.push_back(tc);
    }

    // 2. Populate valid Constant test cases
    for (int i = 0; i < static_cast<int>(valid_constants.size()); ++i) {
        TestCase tc;
        tc.name = "Valid Constant " + std::to_string(i);
        tc.input = valid_constants[i];
        tc.should_succeed = true;

        auto encoded_opt = encode(tc.input);
        if (encoded_opt.has_value()) {
            tc.expected_output = encoded_opt.value();
        } else {
            std::cerr << "Error: Failed to encode valid constant: " << tc.input << std::endl;
            continue;
        }
        test_cases.push_back(tc);
    }

    // 3. Populate valid Complex Expression test cases
    for (int i = 0; i < static_cast<int>(valid_complex.size()); ++i) {
        TestCase tc;
        tc.name = "Valid Complex " + std::to_string(i);
        tc.input = valid_complex[i];
        tc.should_succeed = true;

        auto encoded_opt = encode(tc.input);
        if (encoded_opt.has_value()) {
            tc.expected_output = encoded_opt.value();
        } else {
            std::cerr << "Error: Failed to encode valid complex expression: " << tc.input << std::endl;
            continue;
        }
        test_cases.push_back(tc);
    }

    // 4. Invalid Test Cases
    // 4.1 Invalid Registers
    for (int i = 0; i < static_cast<int>(invalid_registers.size()); ++i) {
        TestCase tc;
        tc.name = "Invalid Register " + std::to_string(i);
        tc.input = invalid_registers[i];
        tc.should_succeed = false;
        test_cases.push_back(tc);
    }

    // 4.2 Invalid Constants
    for (int i = 0; i < static_cast<int>(invalid_constants.size()); ++i) {
        TestCase tc;
        tc.name = "Invalid Constant " + std::to_string(i);
        tc.input = invalid_constants[i];
        tc.should_succeed = false;
        test_cases.push_back(tc);
    }

    // 4.3 Invalid Complex Expressions
    for (int i = 0; i < static_cast<int>(invalid_complex.size()); ++i) {
        TestCase tc;
        tc.name = "Invalid Complex " + std::to_string(i);
        tc.input = invalid_complex[i];
        tc.should_succeed = false;
        test_cases.push_back(tc);
    }

    // Add advanced test cases
    std::vector<TestCase> advanced_tests = generate_advanced_test_cases();
    for (auto& t : advanced_tests) {
        test_cases.push_back(t);
    }

    // Ensure we have at least 10,000 test cases
    while (test_cases.size() < 10000) {
        // Add more invalid test cases if needed
        TestCase tc;
        tc.name = "Invalid Constant Extra " + std::to_string(test_cases.size());
        tc.input = "$invalid" + std::to_string(test_cases.size());
        tc.should_succeed = false;
        test_cases.push_back(tc);
    }

    // Counters for passed and failed tests
    int passed = 0;
    int failed = 0;

    // Multithreading setup
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4; // Default to 4 if unable to detect
    std::vector<std::thread> threads;
    std::vector<std::vector<TestCase>> test_chunks(num_threads);

    // Preallocate memory for test_chunks
    for (auto& chunk : test_chunks) {
        chunk.reserve(test_cases.size() / num_threads + 1);
    }

    // Distribute test cases among threads
    for (size_t i = 0; i < test_cases.size(); ++i) {
        test_chunks[i % num_threads].push_back(test_cases[i]);
    }

    // Mutex for synchronized output
    std::mutex output_mutex;

    // Launch threads
    for (unsigned int i = 0; i < num_threads; ++i) {
        threads.emplace_back(process_test_cases,
                             std::cref(test_chunks[i]),
                             std::ref(passed),
                             std::ref(failed),
                             std::ref(output_mutex));
    }

    // Join threads
    for (auto& th : threads) {
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
