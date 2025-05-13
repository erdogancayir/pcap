#include "packet_handler.h"
#include "signal_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pcap.h>

/**
 * Sniffer thread entry point.
 * Captures packets from a live network interface using libpcap, filters them (TCP/UDP),
 * and enqueues them for processing.
 *
 * @param arg Pointer to packet_queue_t containing configuration and shared queue.
 * @return NULL
 */
void *sniffer_thread(void *arg)
{
    packet_queue_t *queue = (packet_queue_t *)arg;
    char errbuf[PCAP_ERRBUF_SIZE];
    
    // Open a live capture session on the given network interface
    pcap_t *handle = pcap_open_live(queue->cli_config->interface_or_file, BUFSIZ, 1, 100, errbuf);
    if (!handle) {
        fprintf(stderr, "pcap_open_live() failed: %s\n", errbuf);
        packet_queue_mark_done(queue);  // Mark queue as done before exiting
        pthread_exit((void *)1);  // Exit with non-zero status to indicate failure
    }

    // Store the handle globally so it can be closed on signal (e.g., SIGINT)
    set_pcap_handle(handle);

    // Compile a BPF filter to capture only TCP or UDP packets
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, "tcp or udp", 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "pcap_compile() failed: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        packet_queue_mark_done(queue);  // Mark queue as done before exiting
        pthread_exit((void *)1);  // Exit with non-zero status to indicate failure
    }

    // Apply the filter to the capture session
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "pcap_setfilter() failed: %s\n", pcap_geterr(handle));
        pcap_freecode(&fp);
        pcap_close(handle);
        packet_queue_mark_done(queue);  // Mark queue as done before exiting
        pthread_exit((void *)1);  // Exit with non-zero status to indicate failure
    }

    pcap_freecode(&fp);

    // Start the packet capture loop — this blocks and invokes packet_handler() for each packet
    pcap_loop(handle, 0, packet_handler, (u_char *)queue);

    // Close the capture session after loop ends (e.g., via pcap_breakloop or EOF)
    pcap_close(handle);

    // Notify consumer thread that capture is complete
    packet_queue_mark_done(queue);

    LOG_INFO("Sniffer thread finished.");
    pthread_exit(NULL);  // Exit with success status
}