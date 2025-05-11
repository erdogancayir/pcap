#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int input_handle_interface(packet_queue_t *packet_queue)
{
    DEBUG("[INFO] Starting input_handle_interface...");

    pthread_t producer_sniffer_tid;
    int ret = pthread_create(&producer_sniffer_tid, NULL, sniffer_thread, (void *)packet_queue);
    if (ret != 0) {
        fprintf(stderr, "Error creating sniffer thread: %s\n", strerror(ret));
        return -1;
    }

    pthread_t packet_writer_tid;
    ret = pthread_create(&packet_writer_tid, NULL, packet_writer_thread, (void *)packet_queue);
    if (ret != 0) {
        fprintf(stderr, "Error creating consumer thread: %s\n", strerror(ret));
        return -1;
    }

    // Wait for the threads to finish
    pthread_join(producer_sniffer_tid, NULL);
    pthread_join(packet_writer_tid, NULL);

    return EXIT_SUCCESS;
}