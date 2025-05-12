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

    tcp_tracker_set_output_file(fp); // bağlantı kapanış loglarını da bu dosyaya yaz

    DEBUG("[INFO] Output thread started. Writing to: %s", cli_config->output_file);

    int counter = 0;
    while (1) {

        if (counter == 1000) {
            // signal for waiting thread pthread_cond_wait
            pthread_cond_signal(&packet_queue->not_empty);
            break;
        }
        counter++;

        DEBUG("-- %d", counter);
        captured_packet_t pkt;
        if (packet_queue_dequeue(packet_queue, &pkt) != 0) {
            continue;
        }
        // MAC, IP, PORT bilgileri
        fprintf(fp, "MAC: %s -> %s\n", pkt.src_mac, pkt.dst_mac);
        fprintf(fp, "IP : %s -> %s\n", pkt.src_ip, pkt.dst_ip);
        fprintf(fp, "PORT: %u -> %u\n", pkt.src_port, pkt.dst_port);

        // HTTP varsa
        if (pkt.is_http) {
            fprintf(fp, "HTTP HOST       : %s\n", pkt.host);
            fprintf(fp, "HTTP USER-AGENT : %s\n", pkt.user_agent);
        }

        fprintf(fp, "---------------------------------------\n");
        fflush(fp);  // canlı loglama için
    }

    DEBUG("[INFO] Output thread exiting.");
    fclose(fp);
    pthread_exit(NULL);
}
