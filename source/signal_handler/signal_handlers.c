// signal_handler.c
#include "signal_handler.h"
#include "debug_mode.h"
#include <stdio.h>

volatile sig_atomic_t stop_capture = 0;   // Global flag to indicate SIGINT (safe for signal context)
static pcap_t *g_pcap_handle = NULL;     // Global pointer to active pcap handle, used to break the loop

/**
 * Sets the global pcap handle for later use in signal handling.
 * This is necessary so the signal handler can safely stop packet capturing.
 *
 * @param handle Pointer to an active pcap_t session.
 */
void set_pcap_handle(pcap_t *handle)
{
    g_pcap_handle = handle;
    if (g_pcap_handle == NULL) {
        LOG_ERROR("Failed to set pcap handle.");
        return;
    }

    LOG_DEBUG("pcap handle set successfully.");
}

/**
 * SIGINT (Ctrl+C) signal handler.
 * Called asynchronously by the OS when SIGINT is received.
 * It sets a global flag and breaks the pcap loop if active.
 *
 * @param signo Signal number (unused).
 */
void sigint_handler(int signo)
{
    (void)signo;  // Silence unused parameter warning
    stop_capture = 1;

    LOG_WARN("\n📡 SIGINT detected, stopping... 📡");

    // Gracefully break the pcap loop if capture is running
    if (g_pcap_handle)
    {
        LOG_WARN("Stopping pcap loop...");
        pcap_breakloop(g_pcap_handle);
    }

    LOG_INFO("Signal handler finished.");
}