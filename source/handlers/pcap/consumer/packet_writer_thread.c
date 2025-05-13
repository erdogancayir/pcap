#include "packet_queue.h"
#include "tcp_tracker.h"
#include "debug_mode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Writer thread function.
 * Continuously dequeues captured packets and writes their details
 * (MAC, IP, Port, and optionally HTTP headers) to the output file.
 *
 * Flushes the output file periodically for safety.
 *
 * @param arg Pointer to packet_queue_t shared by producer and consumer.
 * @return NULL
 */
void *writer_thread(void *arg)
{
    packet_queue_t *packet_queue = (packet_queue_t *)arg;
    const cli_config_t *cli_config = packet_queue->cli_config;

    // Open the output file for writing
    FILE *fp = fopen(cli_config->output_file, "w");
    if (!fp)
    {
        perror("fopen (output_file)");
        pthread_exit(NULL);
    }

    // Set output file for optional TCP connection tracking log
    tcp_tracker_set_output_file(fp);

    // Main loop: consume packets from the queue and write to file
    while (1)
    {
        captured_packet_t pkt;

        LOG_INFO("🔍 Writer thread: Waiting for packets to write...");
        // Dequeue packet; if return -1, queue is done and empty
        int status = packet_queue_dequeue(packet_queue, &pkt);
        if (status == -1)
            break;  // queue done and empty

        // Write MAC, IP, and port info
        fprintf(fp, "MAC: %s -> %s\n", pkt.src_mac, pkt.dst_mac);
        fprintf(fp, "IP : %s -> %s\n", pkt.src_ip, pkt.dst_ip);
        fprintf(fp, "PORT: %u -> %u\n", pkt.src_port, pkt.dst_port);

        // Write HTTP headers if available
        if (pkt.is_http)
        {
            fprintf(fp, "HTTP HOST       : %s\n", pkt.host);
            fprintf(fp, "HTTP USER-AGENT : %s\n", pkt.user_agent);
        }
        fprintf(fp, "---------------------------------------\n");

        // Flush every 100 packets to avoid data loss in case of crash
        static int flush_counter = 0;
        if (++flush_counter % 100 == 0)
        {
            fflush(fp);
            flush_counter = 0;
        }
    }

    // Close the output file and exit the thread
    fclose(fp);
    pthread_exit(NULL);
}