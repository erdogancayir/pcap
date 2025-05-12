#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int input_handle_interface(packet_queue_t *packet_queue)
{
    LOG_INFO("🔍 Capturing live packets from interface: %s 🔍", packet_queue->cli_config->interface_or_file);

    pthread_t packet_writer_tid;
    int ret = pthread_create(&packet_writer_tid, NULL, packet_writer_thread, (void *)packet_queue);
    if (ret != 0) {
        LOG_ERROR("Error creating consumer thread: %s", strerror(ret));
        return -1;
    }
    
    pthread_t producer_sniffer_tid;
    ret = pthread_create(&producer_sniffer_tid, NULL, sniffer_thread, (void *)packet_queue);
    if (ret != 0) {
        LOG_ERROR("Error creating sniffer thread: %s", strerror(ret));
        return -1;
    }

    // Wait for the threads to finish
    pthread_join(producer_sniffer_tid, NULL);
    pthread_join(packet_writer_tid, NULL);

    LOG_INFO("Finished input_handle_interface.");
    return EXIT_SUCCESS;
}