#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#define TEST_PCAP_FILE "fixtures/test_traffic.pcap"
#define NUM_PACKETS 100

// Function to create a TCP packet with HTTP content
void create_http_packet(u_char **packet, size_t *len, int is_request) {
    struct ether_header *eth;
    struct ip *ip;
    struct tcphdr *tcp;
    char *payload;
    int payload_len;
    
    // Calculate total length
    *len = sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr);
    
    // Add HTTP payload
    if (is_request) {
        const char *http_req = 
            "GET / HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "User-Agent: TestClient/1.0\r\n"
            "\r\n";
        payload_len = strlen(http_req);
        *len += payload_len;
    } else {
        const char *http_resp = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!";
        payload_len = strlen(http_resp);
        *len += payload_len;
    }
    
    // Allocate packet buffer
    *packet = malloc(*len);
    if (!*packet) {
        *len = 0;
        return;
    }
    
    // Set up Ethernet header
    eth = (struct ether_header *)*packet;
    memset(eth->ether_dhost, 0xAA, ETH_ALEN);
    memset(eth->ether_shost, 0xBB, ETH_ALEN);
    eth->ether_type = htons(ETHERTYPE_IP);
    
    // Set up IP header
    ip = (struct ip *)(*packet + sizeof(struct ether_header));
    ip->ip_v = 4;
    ip->ip_hl = 5;
    ip->ip_tos = 0;
    ip->ip_len = htons(*len - sizeof(struct ether_header));
    ip->ip_id = htons(12345);
    ip->ip_off = 0;
    ip->ip_ttl = 64;
    ip->ip_p = IPPROTO_TCP;
    ip->ip_src.s_addr = inet_addr("192.168.1.1");
    ip->ip_dst.s_addr = inet_addr("192.168.1.2");
    ip->ip_sum = 0; // Let libpcap handle checksum
    
    // Set up TCP header
    tcp = (struct tcphdr *)((u_char *)ip + sizeof(struct ip));
    tcp->th_sport = htons(is_request ? 12345 : 80);
    tcp->th_dport = htons(is_request ? 80 : 12345);
    tcp->th_seq = htonl(123456789);
    tcp->th_ack = htonl(0);
    tcp->th_off = 5;
    tcp->th_flags = is_request ? TH_SYN : TH_ACK;
    tcp->th_win = htons(65535);
    tcp->th_sum = 0; // Let libpcap handle checksum
    tcp->th_urp = 0;
    
    // Add HTTP payload
    payload = (char *)((u_char *)tcp + sizeof(struct tcphdr));
    if (is_request) {
        memcpy(payload, 
            "GET / HTTP/1.1\r\n"
            "Host: example.com\r\n"
            "User-Agent: TestClient/1.0\r\n"
            "\r\n", payload_len);
    } else {
        memcpy(payload,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 13\r\n"
            "\r\n"
            "Hello, World!", payload_len);
    }
}

int main(void) {
    pcap_t *pcap;
    pcap_dumper_t *dumper;
    struct pcap_pkthdr pkthdr;
    u_char *packet = NULL;
    size_t len;
    int i;
    
    // Create pcap file
    pcap = pcap_open_dead(DLT_EN10MB, 65535);
    if (!pcap) {
        fprintf(stderr, "Failed to create pcap handle\n");
        return 1;
    }
    
    dumper = pcap_dump_open(pcap, TEST_PCAP_FILE);
    if (!dumper) {
        fprintf(stderr, "Failed to create pcap file: %s\n", pcap_geterr(pcap));
        pcap_close(pcap);
        return 1;
    }
    
    // Generate packets
    srand(time(NULL));
    for (i = 0; i < NUM_PACKETS; i++) {
        // Create packet
        create_http_packet(&packet, &len, i % 2 == 0);
        if (len == 0 || !packet) {
            fprintf(stderr, "Failed to create packet\n");
            continue;
        }
        
        // Set packet header
        pkthdr.ts.tv_sec = time(NULL);
        pkthdr.ts.tv_usec = 0;
        pkthdr.caplen = len;
        pkthdr.len = len;
        
        // Write packet to file
        pcap_dump((u_char *)dumper, &pkthdr, packet);
        free(packet);
        packet = NULL;
    }
    
    // Cleanup
    pcap_dump_close(dumper);
    pcap_close(pcap);
    
    printf("Generated test pcap file: %s\n", TEST_PCAP_FILE);
    return 0;
} 