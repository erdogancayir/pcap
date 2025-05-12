#include "packet_handler.h"
#include "signal_handler.h"
#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int input_handle_pcap_file(packet_queue_t *packet_queue)
{
    LOG_INFO("Reading from pcap file: %s", packet_queue->cli_config->interface_or_file);

    pthread_t consumer_writer_tid;
    int ret = pthread_create(&consumer_writer_tid, NULL, writer_thread, (void *)packet_queue);
    if (ret != 0) {
        LOG_ERROR("Error creating consumer thread: %s", strerror(ret));
        return -1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline(packet_queue->cli_config->interface_or_file, errbuf);
    if (!handle) {
        LOG_ERROR("pcap_open_offline() failed: %s", errbuf);
        return -1;
    }
    
    set_pcap_handle(handle);
    
    struct bpf_program fp;
    char *filter_exp = "tcp or udp";

    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        LOG_ERROR("pcap_compile() failed: %s", pcap_geterr(handle));
        pcap_close(handle);
        return -1;
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        LOG_ERROR("pcap_setfilter() failed: %s", pcap_geterr(handle));
        pcap_freecode(&fp);
        pcap_close(handle);
        return -1;
    }

    pcap_freecode(&fp);

    // No producer thread needed — we're using pcap_loop() in the main thread
    pcap_loop(handle, 0, packet_handler, (u_char *)packet_queue);
    pcap_close(handle);

    DEBUG("pcap_loop() finished.");

    packet_queue_mark_done(packet_queue);

    // Wait for consumer thread to finish
    pthread_join(consumer_writer_tid, NULL);

    DEBUG("Finished processing pcap file.");
    return EXIT_SUCCESS;
}