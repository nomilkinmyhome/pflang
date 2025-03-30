#ifndef PFLANG_TEST_FRAMEWORK_H
#define PFLANG_TEST_FRAMEWORK_H

#include <stdbool.h>

// Test statistics structure
typedef struct {
    int total;
    int passed;
    int failed;
} TestStats;

// Initialize test statistics
void init_test_stats(TestStats* stats);

// Print test results
void print_test_results(TestStats* stats);

// Assert functions
void assert_true(bool condition, const char* message, TestStats* stats, const char* file, int line);
void assert_false(bool condition, const char* message, TestStats* stats, const char* file, int line);
void assert_equal_int(int expected, int actual, const char* message, TestStats* stats, const char* file, int line);
void assert_equal_string(const char* expected, const char* actual, const char* message, TestStats* stats, const char* file, int line);

// Macros for easier use
#define ASSERT_TRUE(condition, message) assert_true(condition, message, &stats, __FILE__, __LINE__)
#define ASSERT_FALSE(condition, message) assert_false(condition, message, &stats, __FILE__, __LINE__)
#define ASSERT_EQUAL_INT(expected, actual, message) assert_equal_int(expected, actual, message, &stats, __FILE__, __LINE__)
#define ASSERT_EQUAL_STRING(expected, actual, message) assert_equal_string(expected, actual, message, &stats, __FILE__, __LINE__)

#endif // PFLANG_TEST_FRAMEWORK_H
