#include "../test_framework.h"
#include "../../include/packet_handler.h"
#include "../../include/packet_queue.h"
#include "../../include/thread_safe_counter.h"
#include <pcap.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_PCAP_FILE "fixtures/test_traffic.pcap"
#define MAX_PACKET_SIZE 65536

typedef struct {
    int tcp_count;
    int udp_count;
    int http_count;
    thread_safe_counter_t counter;
} PacketStats;

void process_packet(u_char *user, const struct pcap_pkthdr *h, const u_char *packet) {
    PacketStats *stats = (PacketStats *)user;
    struct ether_header *eth_header;
    struct ip *ip_header;
    struct tcphdr *tcp_header;
    
    // Skip if packet is too small
    if (h->len < sizeof(struct ether_header)) {
        return;
    }
    
    eth_header = (struct ether_header *)packet;
    
    // Check if it's an IP packet
    if (ntohs(eth_header->ether_type) != ETHERTYPE_IP) {
        return;
    }
    
    // Skip if packet is too small for IP header
    if (h->len < sizeof(struct ether_header) + sizeof(struct ip)) {
        return;
    }
    
    ip_header = (struct ip *)(packet + sizeof(struct ether_header));
    
    // Count TCP and UDP packets
    if (ip_header->ip_p == IPPROTO_TCP) {
        stats->tcp_count++;
        
        // Skip if packet is too small for TCP header
        if (h->len < sizeof(struct ether_header) + sizeof(struct ip) + sizeof(struct tcphdr)) {
            return;
        }
        
        tcp_header = (struct tcphdr *)((u_char *)ip_header + sizeof(struct ip));
        
        // Check for HTTP traffic (port 80 or 8080)
        if (ntohs(tcp_header->th_dport) == 80 || ntohs(tcp_header->th_dport) == 8080 ||
            ntohs(tcp_header->th_sport) == 80 || ntohs(tcp_header->th_sport) == 8080) {
            stats->http_count++;
        }
    } else if (ip_header->ip_p == IPPROTO_UDP) {
        stats->udp_count++;
    }
    
    thread_safe_counter_increment(&stats->counter);
}

void test_pcap_file_processing(void) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    PacketStats stats = {0, 0, 0, {0}};
    
    // Initialize counter
    thread_safe_counter_init(&stats.counter);
    
    // Open pcap file
    handle = pcap_open_offline(TEST_PCAP_FILE, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Error opening pcap file: %s\n", errbuf);
        TEST_ASSERT(0, "Should be able to open test pcap file");
        return;
    }
    
    // Process packets
    int result = pcap_loop(handle, 0, process_packet, (u_char *)&stats);
    if (result < 0) {
        fprintf(stderr, "Error processing packets: %s\n", pcap_geterr(handle));
        TEST_ASSERT(0, "Should process packets without error");
        pcap_close(handle);
        return;
    }
    
    // Verify results
    TEST_ASSERT(stats.tcp_count > 0, "Should find TCP packets");
    TEST_ASSERT(stats.udp_count >= 0, "UDP packet count should be non-negative");
    TEST_ASSERT(stats.http_count >= 0, "HTTP packet count should be non-negative");
    TEST_ASSERT(thread_safe_counter_get(&stats.counter) > 0, 
                "Should process at least one packet");
    
    printf("Processed %d TCP packets, %d UDP packets, %d HTTP packets\n",
           stats.tcp_count, stats.udp_count, stats.http_count);
    
    // Cleanup
    pcap_close(handle);
}

void test_pcap_file_invalid(void) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    // Try to open non-existent file
    handle = pcap_open_offline("nonexistent.pcap", errbuf);
    TEST_ASSERT(handle == NULL, "Should fail to open non-existent pcap file");
}

void test_pcap_file_corrupted(void) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    // Create a corrupted pcap file
    FILE *f = fopen("fixtures/corrupted.pcap", "w");
    TEST_ASSERT(f != NULL, "Should be able to create corrupted pcap file");
    fwrite("CORRUPTED", 1, 9, f);
    fclose(f);
    
    // Try to open corrupted file
    handle = pcap_open_offline("fixtures/corrupted.pcap", errbuf);
    TEST_ASSERT(handle == NULL, "Should fail to open corrupted pcap file");
    
    // Cleanup
    remove("fixtures/corrupted.pcap");
}

int main(void) {
    init_test_suite();
    
    printf("Running pcap processing tests...\n");
    test_pcap_file_processing();
    test_pcap_file_invalid();
    test_pcap_file_corrupted();
    
    print_test_summary();
    return test_stats.failed_tests > 0 ? 1 : 0;
} 