#include "packet_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include "debug_mode.h"
#include <netinet/udp.h>

void *packet_writer_thread(void *arg)
{
    packet_queue_t *queue = (packet_queue_t *)arg;
    cli_config_t *config = queue->cli_config;

    FILE *output = fopen(config->output_file, "w");
    if (!output) {
        perror("fopen");
        pthread_exit(NULL);
    }

    while (1)
    {
        captured_packet_t *pkt = packet_queue_pop(queue);
        
        struct ether_header *eth = (struct ether_header *)pkt->packet_data;
        if (ntohs(eth->ether_type) != ETHERTYPE_IP) {
            free(pkt->packet_data); free(pkt); continue;
        }

        struct ip *ip_header = (struct ip *)(pkt->packet_data + sizeof(struct ether_header));
        int ip_header_len = ip_header->ip_hl * 4;

        char src_ip[INET_ADDRSTRLEN];
        char dst_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(ip_header->ip_src), src_ip, INET_ADDRSTRLEN);
        inet_ntop(AF_INET, &(ip_header->ip_dst), dst_ip, INET_ADDRSTRLEN);

        uint16_t src_port = 0, dst_port = 0;
        const u_char *payload = NULL;
        int payload_len = 0;

        if (ip_header->ip_p == IPPROTO_TCP)
        {
            DEBUG("*** TCP PACKET ***");
        }
        else if (ip_header->ip_p == IPPROTO_UDP)
        {
            DEBUG("*** UDP PACKET ***");
        }
        else
        {
            DEBUG("*** %s ***", ip_header->ip_p);
        }

        fflush(output);
        free(pkt->packet_data);
        free(pkt);
    }
    
    fclose(output);
    return NULL;
}   