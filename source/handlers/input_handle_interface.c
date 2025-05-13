#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Handles live packet capture from a network interface.
 * Launches two threads: one for sniffing (producer) and another for writing output (consumer).
 *
 * @param packet_queue Shared structure used for passing data between producer and consumer.
 * @return EXIT_SUCCESS on success, -1 on error.
 */
int input_handle_interface(packet_queue_t *packet_queue)
{
    LOG_INFO("🔍 Capturing live packets from interface => %s 🔍", packet_queue->cli_config->interface_or_file);

    // Start consumer thread: continuously writes parsed packets from the queue to the output file.
    pthread_t consumer_writer_tid;
    int ret = pthread_create(&consumer_writer_tid, NULL, writer_thread, (void *)packet_queue);
    if (ret != 0) {
        LOG_ERROR("Error creating consumer thread: %s", strerror(ret));
        return -1;
    }
    
    // Start producer thread: captures live packets from the specified interface and enqueues them.
    pthread_t producer_sniffer_tid;
    ret = pthread_create(&producer_sniffer_tid, NULL, sniffer_thread, (void *)packet_queue);
    if (ret != 0) {
        LOG_ERROR("Error creating sniffer thread: %s", strerror(ret));

        pthread_mutex_lock(&packet_queue->mutex);
        packet_queue->done = 1;
        pthread_cond_signal(&packet_queue->not_empty); // Wake up writer if waiting
        pthread_mutex_unlock(&packet_queue->mutex);
        
        pthread_join(consumer_writer_tid, NULL);

        return -1;
    }

    // Wait for both threads to complete their tasks before exiting.
    pthread_join(producer_sniffer_tid, NULL);
    pthread_join(consumer_writer_tid, NULL);

    LOG_INFO("Finished input_handle_interface.");
    return EXIT_SUCCESS;
}