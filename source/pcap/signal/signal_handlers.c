// signal_handler.c
#include "signal_handler.h"
#include "debug_mode.h"
#include <stdio.h>

volatile sig_atomic_t stop_capture = 0;
static pcap_t *g_pcap_handle = NULL;

void set_pcap_handle(pcap_t *handle)
{
    g_pcap_handle = handle;
    if (g_pcap_handle == NULL) {
        LOG_ERROR("Failed to set pcap handle.");
        return;
    }

    LOG_DEBUG("pcap handle set successfully.");
}

void sigint_handler(int signo) {
    (void)signo;
    stop_capture = 1;

    LOG_WARN("SIGINT detected, stopping...");

    if (g_pcap_handle) {
        LOG_WARN("Stopping pcap loop...");
        pcap_breakloop(g_pcap_handle);
    }

    LOG_INFO("Signal handler finished.");
}