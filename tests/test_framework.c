#include "test_framework.h"

// Test result tracking
TestStats test_stats = {0, 0, 0};

void init_test_suite(void) {
    test_stats.total_tests = 0;
    test_stats.passed_tests = 0;
    test_stats.failed_tests = 0;
}

void print_test_summary(void) {
    printf("\nTest Summary:\n");
    printf("Total tests: %d\n", test_stats.total_tests);
    printf("Passed: %d\n", test_stats.passed_tests);
    printf("Failed: %d\n", test_stats.failed_tests);
}
