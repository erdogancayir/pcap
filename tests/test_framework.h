#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define ETH_ALEN 6

// Test result tracking
typedef struct {
    int total_tests;
    int passed_tests;
    int failed_tests;
} TestStats;

extern TestStats test_stats;

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

// Test suite management
void init_test_suite(void);
void print_test_summary(void);

// Thread safety testing utilities
typedef struct {
    pthread_mutex_t mutex;
    int value;
} ThreadSafeCounter;

void init_thread_safe_counter(ThreadSafeCounter *counter);
void increment_counter(ThreadSafeCounter *counter);
int get_counter_value(ThreadSafeCounter *counter);

// Packet generation utilities
unsigned char* create_test_ethernet_frame(size_t *len);
unsigned char* create_test_ip_packet(size_t *len);
unsigned char* create_test_tcp_packet(size_t *len);
unsigned char* create_test_http_packet(size_t *len);

#endif // TEST_FRAMEWORK_H 