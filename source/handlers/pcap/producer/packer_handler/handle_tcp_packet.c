#include <net/ethernet.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <stdio.h>

#include "captured_packet.h"
#include "packet_handler.h"
#include "packet_queue.h"
#include "tcp_tracker.h"
#include "debug_mode.h"

#include "my_libc.h"

/**
 * Processes a TCP packet: extracts Ethernet, IP, TCP, and HTTP data (if any),
 * populates a `captured_packet_t` struct, performs TCP connection tracking,
 * and enqueues the packet for further processing/writing.
 *
 * @param packet      Raw packet data captured by libpcap.
 * @param header      Metadata for the packet (timestamp, length).
 * @param queue       Pointer to the shared producer-consumer queue.
 * @param ip_header   Parsed IP header (already extracted by caller).
 * @param ip_header_len Length of the IP header in bytes.
 */
void handle_tcp_packet(const u_char *packet, const struct pcap_pkthdr *header,
                       packet_queue_t *queue, const struct ip *ip_header, int ip_header_len)
{
    // Extract Ethernet header (first 14 bytes)
    const struct ether_header *eth = (struct ether_header *)packet;
    
    // Sanity check: ensure it's an IP packet
    if (ntohs(eth->ether_type) != ETHERTYPE_IP)
    return;

    // Extract TCP header (located after IP header)
    const struct tcphdr *tcp_header = (struct tcphdr *)((u_char *)ip_header + ip_header_len);
    int tcp_header_len = tcp_header->th_off * 4;

    // Compute pointer to TCP payload and its length
    const u_char *payload = (u_char *)tcp_header + tcp_header_len;
    int payload_len = header->caplen - (payload - packet);

    // Initialize and zero out packet structure
    captured_packet_t captured_packet;
    my_memset(&captured_packet, 0, sizeof(captured_packet));

    // Populate MAC addresses
    snprintf(captured_packet.src_mac, sizeof(captured_packet.src_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2],
             eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);

    snprintf(captured_packet.dst_mac, sizeof(captured_packet.dst_mac), "%02x:%02x:%02x:%02x:%02x:%02x",
             eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2],
             eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);

    // Populate IP addresses
    my_strncpy(captured_packet.src_ip, inet_ntoa(ip_header->ip_src), sizeof(captured_packet.src_ip) - 1);
    my_strncpy(captured_packet.dst_ip, inet_ntoa(ip_header->ip_dst), sizeof(captured_packet.dst_ip) - 1);

    // Populate TCP ports
    captured_packet.src_port = ntohs(tcp_header->th_sport);
    captured_packet.dst_port = ntohs(tcp_header->th_dport);

    // Track TCP connection statistics (SYN, FIN, etc.)
    tcp_tracker_process_packet(
        captured_packet.src_ip,
        captured_packet.dst_ip,
        captured_packet.src_port,
        captured_packet.dst_port,
        tcp_header->th_flags // TH_FIN, TH_ACK, TH_RST vs.
    );

    // If payload starts with HTTP methods, attempt to extract HTTP headers
    if (payload_len > 0 && (my_memcmp(payload, "GET ", 4) == 0 || my_memcmp(payload, "POST ", 5) == 0))
    {
        captured_packet.is_http = 1;

        // Extract "Host" header
        const char *host = my_strnstr((const char *)payload, "Host: ", payload_len);
        if (host)
            sscanf(host, "Host: %255[^\r\n]", captured_packet.host);

        // Extract "User-Agent" header
        const char *ua = my_strnstr((const char *)payload, "User-Agent: ", payload_len);
        if (ua)
            sscanf(ua, "User-Agent: %511[^\r\n]", captured_packet.user_agent);
    }

    // Enqueue the structured packet for writing/analysis
    packet_queue_enqueue(queue, &captured_packet);
}