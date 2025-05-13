#include "test_framework.h"
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/ethernet.h>

TestStats test_stats = {0, 0, 0};

void init_test_suite(void) {
    test_stats.total_tests = 0;
    test_stats.passed_tests = 0;
    test_stats.failed_tests = 0;
}

void print_test_summary(void) {
    printf("\nTest Summary:\n");
    printf("Total Tests: %d\n", test_stats.total_tests);
    printf("Passed: %d\n", test_stats.passed_tests);
    printf("Failed: %d\n", test_stats.failed_tests);
    printf("Success Rate: %.2f%%\n", 
           (float)test_stats.passed_tests / test_stats.total_tests * 100);
}

// Thread safety testing implementation
void init_thread_safe_counter(ThreadSafeCounter *counter) {
    pthread_mutex_init(&counter->mutex, NULL);
    counter->value = 0;
}

void increment_counter(ThreadSafeCounter *counter) {
    pthread_mutex_lock(&counter->mutex);
    counter->value++;
    pthread_mutex_unlock(&counter->mutex);
}

int get_counter_value(ThreadSafeCounter *counter) {
    int value;
    pthread_mutex_lock(&counter->mutex);
    value = counter->value;
    pthread_mutex_unlock(&counter->mutex);
    return value;
}

// Packet generation utilities
unsigned char* create_test_ethernet_frame(size_t *len) {
    *len = sizeof(struct ether_header) + 1500; // Max Ethernet payload
    unsigned char *frame = malloc(*len);
    if (!frame) return NULL;

    struct ether_header *eth = (struct ether_header *)frame;
    // Set some test MAC addresses
    memset(eth->ether_dhost, 0xAA, ETH_ALEN);
    memset(eth->ether_shost, 0xBB, ETH_ALEN);
    eth->ether_type = htons(ETHERTYPE_IP);

    return frame;
}

unsigned char* create_test_ip_packet(size_t *len) {
    *len = sizeof(struct ip) + 1500; // Max IP payload
    unsigned char *packet = malloc(*len);
    if (!packet) return NULL;

    struct ip *ip_header = (struct ip *)packet;
    ip_header->ip_v = 4;
    ip_header->ip_hl = 5;
    ip_header->ip_tos = 0;
    ip_header->ip_len = htons(*len);
    ip_header->ip_id = htons(12345);
    ip_header->ip_off = 0;
    ip_header->ip_ttl = 64;
    ip_header->ip_p = IPPROTO_TCP;
    ip_header->ip_src.s_addr = inet_addr("192.168.1.1");
    ip_header->ip_dst.s_addr = inet_addr("192.168.1.2");

    return packet;
}

unsigned char* create_test_tcp_packet(size_t *len) {
    *len = sizeof(struct tcphdr) + 1500; // Max TCP payload
    unsigned char *packet = malloc(*len);
    if (!packet) return NULL;

    struct tcphdr *tcp = (struct tcphdr *)packet;
    tcp->source = htons(12345);
    tcp->dest = htons(80);
    tcp->seq = htonl(123456789);
    tcp->ack_seq = htonl(0);
    tcp->doff = 5;
    tcp->fin = 0;
    tcp->syn = 1;
    tcp->rst = 0;
    tcp->psh = 0;
    tcp->ack = 0;
    tcp->urg = 0;
    tcp->window = htons(65535);
    tcp->check = 0;
    tcp->urg_ptr = 0;

    return packet;
}

unsigned char* create_test_http_packet(size_t *len) {
    const char *http_request = 
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: TestClient/1.0\r\n"
        "\r\n";
    
    *len = strlen(http_request);
    unsigned char *packet = malloc(*len);
    if (!packet) return NULL;

    memcpy(packet, http_request, *len);
    return packet;
} 