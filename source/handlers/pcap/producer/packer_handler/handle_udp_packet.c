#include <net/ethernet.h>
#include <netinet/udp.h>
#include <stdio.h>

#include "captured_packet.h"
#include "packet_handler.h"
#include "packet_queue.h"

#include "my_libc.h"

/**
 * Processes a UDP packet and populates a `captured_packet_t` structure
 * with source/destination MAC addresses, IP addresses, and ports.
 * The structured packet is then enqueued for writing or further processing.
 *
 * @param packet     Raw packet data, including Ethernet header.
 * @param queue      Pointer to the shared packet queue for producer-consumer communication.
 * @param ip_header  Parsed IP header (already extracted by caller).
 */
void handle_udp_packet(const u_char *packet, packet_queue_t *queue, const struct ip *ip_header)
{
    // Extract Ethernet header from the beginning of the raw packet
    const struct ether_header *eth = (struct ether_header *)packet;

    // Locate UDP header by offsetting past the IP header
    const struct udphdr *udp_header = (struct udphdr *)((u_char *)ip_header + (ip_header->ip_hl * 4));

    // Initialize a captured packet structure and zero it out
    captured_packet_t captured_packet;
    my_memset(&captured_packet, 0, sizeof(captured_packet));

    // Extract and format MAC addresses
    snprintf(captured_packet.src_mac, sizeof(captured_packet.src_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
             eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);

    snprintf(captured_packet.dst_mac, sizeof(captured_packet.dst_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
             eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);

    // Copy IP addresses from parsed IP header
    my_strncpy(captured_packet.src_ip, inet_ntoa(ip_header->ip_src), sizeof(captured_packet.src_ip) - 1);
    my_strncpy(captured_packet.dst_ip, inet_ntoa(ip_header->ip_dst), sizeof(captured_packet.dst_ip) - 1);

    // Extract UDP ports
    captured_packet.src_port = ntohs(udp_header->uh_sport);
    captured_packet.dst_port = ntohs(udp_header->uh_dport);

    // Mark as non-HTTP (we don't inspect UDP payload for HTTP)
    captured_packet.is_http = 0;

    // Enqueue the captured packet for downstream processing
    packet_queue_enqueue(queue, &captured_packet);
}