#include "packet_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#define ETHERNET_HDR_LEN 14

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    packet_queue_t *queue = (packet_queue_t *)args;

    const struct ether_header *eth_header = (const struct ether_header *)packet;
    const struct ip *ip_header = (const struct ip *)(packet + ETHERNET_HDR_LEN);

    if (ip_header == NULL)
        return;

    uint8_t protocol = ip_header->ip_p;
    int ip_header_len = ip_header->ip_hl * 4;

    switch (protocol) {
        case IPPROTO_TCP:
            handle_tcp_packet(packet, header, queue, ip_header, ip_header_len);
            break;
        case IPPROTO_UDP:
            handle_udp_packet(packet, queue, ip_header);
            break;
        default:
            DEBUG("[INFO] Unknown protocol: %u", protocol);
            break;
    }
}