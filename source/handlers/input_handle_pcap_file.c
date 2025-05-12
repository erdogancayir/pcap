#include <stdio.h>
#include <pcap.h>

#include "cli_config.h"
#include "packet_queue.h"

#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pcap.h>

#include "packet_handler.h"

int input_handle_pcap_file(packet_queue_t *packet_queue)
{
    DEBUG("[INFO] Starting input_handle_pcap_file...");

    pthread_t consumer_thread_tid;
    int ret = pthread_create(&consumer_thread_tid, NULL, packet_writer_thread, (void *)packet_queue);
    if (ret != 0) {
        fprintf(stderr, "Error creating consumer thread: %s\n", strerror(ret));
        return -1;
    }

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_offline(packet_queue->cli_config->interface_or_file, errbuf);
    if (!handle) {
        fprintf(stderr, "pcap_open_offline() failed: %s\n", errbuf);
        return -1;
    }

    struct bpf_program fp;
    char *filter_exp = "tcp or udp";

    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "pcap_compile() failed: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        return -1;
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "pcap_setfilter() failed: %s\n", pcap_geterr(handle));
        pcap_freecode(&fp);
        pcap_close(handle);
        return -1;
    }

    pcap_freecode(&fp);

    // No producer thread needed — we're using pcap_loop() in the main thread
    pcap_loop(handle, 0, packet_handler, (u_char *)packet_queue);
    pcap_close(handle);

    // Wait for consumer thread to finish
    pthread_join(consumer_thread_tid, NULL);

    return EXIT_SUCCESS;
}