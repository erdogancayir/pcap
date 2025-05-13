#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

#define COLOR_RESET  "\x1b[0m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_RED    "\x1b[31m"
#define COLOR_YELLOW "\x1b[33m"

#define TEST_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, COLOR_RED "[FAIL] %s:%d: %s" COLOR_RESET "\n", __FILE__, __LINE__, msg); \
        } else { \
            fprintf(stdout, COLOR_GREEN "[PASS] %s" COLOR_RESET "\n", msg); \
        } \
    } while (0)

#endif