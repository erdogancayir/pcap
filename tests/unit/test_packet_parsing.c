#include "../test_framework.h"
#include "../../include/packet_handler.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>\


void test_ethernet_parsing(void) {
    size_t len;
    unsigned char *frame = create_test_ethernet_frame(&len);
    TEST_ASSERT(frame != NULL, "Ethernet frame creation");

    struct ether_header *eth = (struct ether_header *)frame;
    TEST_EQUAL(ntohs(eth->ether_type), ETHERTYPE_IP, "Ethernet type should be IP");
    
    // Test MAC address parsing
    unsigned char expected_dhost[ETH_ALEN] = {0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};
    unsigned char expected_shost[ETH_ALEN] = {0xBB, 0xBB, 0xBB, 0xBB, 0xBB, 0xBB};
    
    TEST_ASSERT(memcmp(eth->ether_dhost, expected_dhost, ETH_ALEN) == 0, 
                "Destination MAC address should match");
    TEST_ASSERT(memcmp(eth->ether_shost, expected_shost, ETH_ALEN) == 0, 
                "Source MAC address should match");

    free(frame);
}

void test_ip_parsing(void) {
    size_t len;
    unsigned char *packet = create_test_ip_packet(&len);
    TEST_ASSERT(packet != NULL, "IP packet creation");

    struct ip *ip_header = (struct ip *)packet;
    TEST_EQUAL(ip_header->ip_v, 4, "IP version should be 4");
    TEST_EQUAL(ip_header->ip_hl, 5, "IP header length should be 5 words");
    TEST_EQUAL(ip_header->ip_p, IPPROTO_TCP, "Protocol should be TCP");
    
    // Test IP addresses
    struct in_addr expected_src, expected_dst;
    inet_aton("192.168.1.1", &expected_src);
    inet_aton("192.168.1.2", &expected_dst);
    
    TEST_ASSERT(ip_header->ip_src.s_addr == expected_src.s_addr, 
                "Source IP should match");
    TEST_ASSERT(ip_header->ip_dst.s_addr == expected_dst.s_addr, 
                "Destination IP should match");

    free(packet);
}

void test_tcp_parsing(void) {
    size_t len;
    unsigned char *packet = create_test_tcp_packet(&len);
    TEST_ASSERT(packet != NULL, "TCP packet creation");

    struct tcphdr *tcp = (struct tcphdr *)packet;
    TEST_EQUAL(ntohs(tcp->source), 12345, "Source port should be 12345");
    TEST_EQUAL(ntohs(tcp->dest), 80, "Destination port should be 80");
    TEST_EQUAL(tcp->syn, 1, "SYN flag should be set");
    TEST_EQUAL(tcp->ack, 0, "ACK flag should not be set");
    TEST_EQUAL(tcp->fin, 0, "FIN flag should not be set");

    free(packet);
}

void test_http_parsing(void) {
    size_t len;
    unsigned char *packet = create_test_http_packet(&len);
    TEST_ASSERT(packet != NULL, "HTTP packet creation");

    // Test HTTP header parsing
    char *host = strstr((char *)packet, "Host: example.com");
    char *user_agent = strstr((char *)packet, "User-Agent: TestClient/1.0");
    
    TEST_ASSERT(host != NULL, "Host header should be present");
    TEST_ASSERT(user_agent != NULL, "User-Agent header should be present");
    TEST_ASSERT(strstr((char *)packet, "GET / HTTP/1.1") != NULL, 
                "HTTP method and path should be correct");

    free(packet);
}

int main(void) {
    init_test_suite();
    
    printf("Running packet parsing tests...\n");
    test_ethernet_parsing();
    test_ip_parsing();
    test_tcp_parsing();
    test_http_parsing();
    
    print_test_summary();
    return test_stats.failed_tests > 0 ? 1 : 0;
} 