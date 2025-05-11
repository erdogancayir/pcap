#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <pcap.h>
#include <netinet/ip.h>
#include "packet_queue.h"

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

// Modüler TCP/UDP işleyicileri
void handle_tcp_packet(const u_char *packet, const struct pcap_pkthdr *header, packet_queue_t *queue, const struct ip *ip_header, int ip_header_len);
void handle_udp_packet(const u_char *packet, packet_queue_t *queue, const struct ip *ip_header);

#endif // PACKET_HANDLER_H