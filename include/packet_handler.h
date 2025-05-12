#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#define ETHERNET_HDR_LEN 14

#include "packet_queue.h"

#include <netinet/ip.h>

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void handle_tcp_packet(const u_char *packet, const struct pcap_pkthdr *header, packet_queue_t *queue, const struct ip *ip_header, int ip_header_len);
void handle_udp_packet(const u_char *packet, packet_queue_t *queue, const struct ip *ip_header);

pcap_t *get_pcap_handle(void);

const char *protocol_to_string(uint8_t proto);

#endif // PACKET_HANDLER_H