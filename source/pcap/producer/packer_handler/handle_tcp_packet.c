#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

#include "packet_queue.h"
#define ETHERNET_HDR_LEN 14


#include <netinet/tcp.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <net/ethernet.h>

#include "packet_handler.h"
#include "captured_packet.h"
#include "packet_queue.h"

#include "debug_mode.h"

#include "my_libc.h"

#define ETHERNET_HDR_LEN 14

#include "tcp_tracker.h" // Eklenmeli

void handle_tcp_packet(const u_char *packet, const struct pcap_pkthdr *header,
                       packet_queue_t *queue, const struct ip *ip_header, int ip_header_len)
{
    const struct ether_header *eth = (struct ether_header *)packet;
    const struct tcphdr *tcp_header = (struct tcphdr *)((u_char *)ip_header + ip_header_len);
    int tcp_header_len = tcp_header->th_off * 4;

    const u_char *payload = (u_char *)tcp_header + tcp_header_len;
    int payload_len = header->caplen - (payload - packet);

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

    // Port
    captured_packet.src_port = ntohs(tcp_header->th_sport);
    captured_packet.dst_port = ntohs(tcp_header->th_dport);

    tcp_tracker_process_packet(
        captured_packet.src_ip,
        captured_packet.dst_ip,
        captured_packet.src_port,
        captured_packet.dst_port,
        tcp_header->th_flags // TH_FIN, TH_ACK, TH_RST vs.
    );

    // HTTP?
    if (payload_len > 0 &&
        (my_memcmp(payload, "GET ", 4) == 0 || my_memcmp(payload, "POST ", 5) == 0)) {
        captured_packet.is_http = 1;

        const char *host = my_strnstr((const char *)payload, "Host: ", payload_len);
        const char *ua   = my_strnstr((const char *)payload, "User-Agent: ", payload_len);

        if (host)
            sscanf(host, "Host: %255[^\r\n]", captured_packet.host);
        if (ua)
            sscanf(ua, "User-Agent: %511[^\r\n]", captured_packet.user_agent);
    }

    packet_queue_enqueue(queue, &captured_packet);

    //packet_queue_debug_dump(queue);
}

void packet_queue_debug_dump(const packet_queue_t *q)
{
    if (!q || !q->buffer) {
        DEBUG("[ERROR] queue or buffer is NULL");
        return;
    }

    DEBUG("[DEBUG] Dumping packet queue (%d items):", q->count);

    int index = q->head;
    for (int i = 0; i < q->count; i++) {
        const captured_packet_t *pkt = &q->buffer[index];

        DEBUG("[%d] %s:%u -> %s:%u %s",
              i,
              pkt->src_ip,
              pkt->src_port,
              pkt->dst_ip,
              pkt->dst_port,
              pkt->is_http ? "[HTTP]" : "");

        index = (index + 1) % PACKET_QUEUE_SIZE;
    }
}