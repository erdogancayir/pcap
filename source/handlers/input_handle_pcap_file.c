#include "packet_handler.h"
#include "signal_handler.h"
#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Handles reading and processing packets from a .pcap file.
 * Spawns a writer thread to consume and persist extracted packet data.
 * Runs packet parsing directly on the main thread via pcap_loop.
 *
 * @param packet_queue Pointer to the shared queue used for producer-consumer communication.
 * @return EXIT_SUCCESS on successful completion, -1 on failure.
 */
int input_handle_pcap_file(packet_queue_t *packet_queue)
{
    LOG_INFO("Reading from pcap file: %s", packet_queue->cli_config->interface_or_file);

    // Start the consumer thread (writer), which will dequeue parsed packet info and write it to the output file.
    pthread_t consumer_writer_tid;
    int ret = pthread_create(&consumer_writer_tid, NULL, writer_thread, (void *)packet_queue);
    if (ret != 0) {
        LOG_ERROR("Error creating consumer thread: %s", strerror(ret));
        return -1;
    }

    // Open the offline pcap file for reading
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline(packet_queue->cli_config->interface_or_file, errbuf);
    if (!handle) {
        LOG_ERROR("pcap_open_offline() failed: %s", errbuf);
        return -1;
    }

    // Save the handle for potential use in cleanup or signal handling
    set_pcap_handle(handle);
    
    // Compile a BPF filter to limit packets to only TCP and UDP
    struct bpf_program fp;
    char *filter_exp = "tcp or udp";

    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        LOG_ERROR("pcap_compile() failed: %s", pcap_geterr(handle));
        pcap_close(handle);
        return -1;
    }

    // Apply the compiled filter to the pcap session
    if (pcap_setfilter(handle, &fp) == -1) {
        LOG_ERROR("pcap_setfilter() failed: %s", pcap_geterr(handle));
        pcap_freecode(&fp);
        pcap_close(handle);
        return -1;
    }

    pcap_freecode(&fp);

    // Begin reading packets in this thread using the provided packet handler callback
    pcap_loop(handle, 0, packet_handler, (u_char *)packet_queue);
    
    // Cleanup the pcap session
    pcap_close(handle);

    DEBUG("pcap_loop() finished.");

    // Notify the writer thread that no more packets will be added to the queue
    packet_queue_mark_done(packet_queue);

    // Wait for the writer thread to finish flushing all data
    pthread_join(consumer_writer_tid, NULL);

    DEBUG("Finished processing pcap file.");
    return EXIT_SUCCESS;
}