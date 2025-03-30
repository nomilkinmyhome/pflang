#include "../include/test_framework.h"
#include <stdio.h>
#include <string.h>

void init_test_stats(TestStats* stats) {
    stats->total = 0;
    stats->passed = 0;
    stats->failed = 0;
}

void print_test_results(TestStats* stats) {
    printf("\nTest Results: %d total, %d passed, %d failed\n", 
           stats->total, stats->passed, stats->failed);
    
    if (stats->failed == 0) {
        printf("All tests passed!\n");
    } else {
        printf("%d tests failed!\n", stats->failed);
    }
}

void assert_true(bool condition, const char* message, TestStats* stats, const char* file, int line) {
    stats->total++;
    
    if (condition) {
        stats->passed++;
        printf("✓ %s\n", message);
    } else {
        stats->failed++;
        printf("✗ %s (at %s:%d)\n", message, file, line);
    }
}

void assert_false(bool condition, const char* message, TestStats* stats, const char* file, int line) {
    assert_true(!condition, message, stats, file, line);
}

void assert_equal_int(int expected, int actual, const char* message, TestStats* stats, const char* file, int line) {
    stats->total++;
    
    if (expected == actual) {
        stats->passed++;
        printf("✓ %s (expected: %d, got: %d)\n", message, expected, actual);
    } else {
        stats->failed++;
        printf("✗ %s (expected: %d, got: %d) (at %s:%d)\n", message, expected, actual, file, line);
    }
}

void assert_equal_string(const char* expected, const char* actual, const char* message, TestStats* stats, const char* file, int line) {
    stats->total++;
    
    if (strcmp(expected, actual) == 0) {
        stats->passed++;
        printf("✓ %s (expected: \"%s\", got: \"%s\")\n", message, expected, actual);
    } else {
        stats->failed++;
        printf("✗ %s (expected: \"%s\", got: \"%s\") (at %s:%d)\n", message, expected, actual, file, line);
    }
}
