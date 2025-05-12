#ifndef DEBUG_MODE_H
#define DEBUG_MODE_H

#include <stdio.h>

#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[1;31m"
#define COLOR_GREEN   "\033[1;32m"
#define COLOR_YELLOW  "\033[1;33m"
#define COLOR_BLUE    "\033[1;34m"
#define COLOR_CYAN    "\033[1;36m"

#ifdef DEBUG_MODE
    #define DEBUG(...) do { \
        fprintf(stderr, COLOR_YELLOW "[DEBUG]" COLOR_RESET " %s:%d: ", __func__, __LINE__); \
        fprintf(stderr, COLOR_CYAN __VA_ARGS__); \
        fprintf(stderr, COLOR_RESET "\n"); \
    } while (0)
#else
    #define DEBUG(...) do {} while (0)
#endif

#define LOG_INFO(fmt, ...)   fprintf(stderr, COLOR_GREEN  "[INFO] " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)   fprintf(stderr, COLOR_YELLOW "[WARN] " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)  fprintf(stderr, COLOR_RED    "[ERROR] " fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)  fprintf(stderr, COLOR_BLUE   "[DEBUG] " fmt COLOR_RESET "\n", ##__VA_ARGS__)

#endif