#ifndef DEBUG_MODE_H
#define DEBUG_MODE_H

#include <stdio.h>

// ANSI renk kodları
#define COLOR_RESET   "\x1b[0m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_BOLD    "\x1b[1m"

#ifdef DEBUG_MODE
    #define DEBUG(...) do { \
        fprintf(stderr, COLOR_YELLOW "[DEBUG]" COLOR_RESET " %s:%d: ", __func__, __LINE__); \
        fprintf(stderr, COLOR_CYAN __VA_ARGS__); \
        fprintf(stderr, COLOR_RESET "\n"); \
    } while (0)
#else
    #define DEBUG(...) do {} while (0)
#endif

#endif