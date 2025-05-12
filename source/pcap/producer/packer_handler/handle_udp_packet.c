#include <net/ethernet.h>
#include <netinet/udp.h>
#include <stdio.h>

#include "captured_packet.h"
#include "packet_handler.h"
#include "packet_queue.h"

#include "my_libc.h"

void handle_udp_packet(const u_char *packet, packet_queue_t *queue, const struct ip *ip_header)
{
    const struct ether_header *eth = (struct ether_header *)packet;
    const struct udphdr *udp_header = (struct udphdr *)((u_char *)ip_header + (ip_header->ip_hl * 4));

    captured_packet_t captured_packet;
    my_memset(&captured_packet, 0, sizeof(captured_packet));

    // MAC
    snprintf(captured_packet.src_mac, sizeof(captured_packet.src_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
             eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);

    snprintf(captured_packet.dst_mac, sizeof(captured_packet.dst_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
             eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);

    // IP
    my_strncpy(captured_packet.src_ip, inet_ntoa(ip_header->ip_src), sizeof(captured_packet.src_ip) - 1);
    my_strncpy(captured_packet.dst_ip, inet_ntoa(ip_header->ip_dst), sizeof(captured_packet.dst_ip) - 1);

    // Ports
    captured_packet.src_port = ntohs(udp_header->uh_sport);
    captured_packet.dst_port = ntohs(udp_header->uh_dport);

    captured_packet.is_http = 0;

    packet_queue_enqueue(queue, &captured_packet);
}