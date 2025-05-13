#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Test result tracking
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestStats;

// External declarations
extern TestStats test_stats;
extern void init_test_suite(void);
extern void print_test_summary(void);

// Test utilities
#define TEST_ASSERT(condition, message) do { \
    test_stats.total_tests++; \
    if (condition) { \
        test_stats.passed_tests++; \
        printf("✓ %s\n", message); \
    } else { \
        test_stats.failed_tests++; \
        printf("✗ %s\n", message); \
    } \
} while(0)

#define TEST_EQUAL(expected, actual, message) \
    TEST_ASSERT((expected) == (actual), message)

#define TEST_STRING_EQUAL(expected, actual, message) \
    TEST_ASSERT(strcmp(expected, actual) == 0, message)

#endif // TEST_FRAMEWORK_H 