#include "packet_queue.h"
#include "tcp_tracker.h"
#include "debug_mode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *packet_writer_thread(void *arg)
{
    packet_queue_t *packet_queue = (packet_queue_t *)arg;
    const cli_config_t *cli_config = packet_queue->cli_config;

    FILE *fp = fopen(cli_config->output_file, "w");
    if (!fp) {
        perror("fopen (output_file)");
        pthread_exit(NULL);
    }

    tcp_tracker_set_output_file(fp);

    while (1) {
        captured_packet_t pkt;

        int status = packet_queue_dequeue(packet_queue, &pkt);
        if (status == -1)
            break;  // queue done and empty

        // MAC, IP, PORT
        fprintf(fp, "MAC: %s -> %s\n", pkt.src_mac, pkt.dst_mac);
        fprintf(fp, "IP : %s -> %s\n", pkt.src_ip, pkt.dst_ip);
        fprintf(fp, "PORT: %u -> %u\n", pkt.src_port, pkt.dst_port);
        if (pkt.is_http) {
            fprintf(fp, "HTTP HOST       : %s\n", pkt.host);
            fprintf(fp, "HTTP USER-AGENT : %s\n", pkt.user_agent);
        }
        fprintf(fp, "---------------------------------------\n");

        static int flush_counter = 0;
        if (++flush_counter % 100 == 0)
        {
            fflush(fp);
            flush_counter = 0;
        }
    }

    fclose(fp);
    pthread_exit(NULL);
}