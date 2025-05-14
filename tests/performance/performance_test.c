#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <pcap.h>
#include "../test_framework.h"

#define device_name "en0"

// Function to get current time in microseconds
static long get_time_usec(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

// Performance test for packet capture initialization
void test_pcap_init_performance(void) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    long start_time, end_time;
    int iterations = 100;
    long total_time = 0;

    printf("\n=== Testing pcap initialization performance ===\n");
    
    for (int i = 0; i < iterations; i++) {
        start_time = get_time_usec();
        
        // Initialize pcap
        handle = pcap_open_live(device_name, BUFSIZ, 1, 1000, errbuf);
        if (handle == NULL) {
            printf("Error opening device: %s\n", errbuf);
            return;
        }
        
        end_time = get_time_usec();
        total_time += (end_time - start_time);
        
        pcap_close(handle);
    }
    
    printf("Average initialization time: %.2f microseconds\n", (float)total_time / iterations);
}

// Performance test for packet processing
void test_packet_processing_performance(void) {
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    struct pcap_pkthdr *header;
    const u_char *packet;
    int res;
    long start_time, end_time;
    int packet_count = 0;
    const int max_packets = 1000;
    
    printf("\n=== Testing packet processing performance ===\n");
    
    handle = pcap_open_live(device_name, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        printf("Error opening device: %s\n", errbuf);
        return;
    }
    
    start_time = get_time_usec();
    
    while (packet_count < max_packets) {
        res = pcap_next_ex(handle, &header, &packet);
        if (res == 0) continue;
        if (res == -1 || res == -2) break;
        
        packet_count++;
    }
    
    end_time = get_time_usec();
    
    printf("Processed %d packets in %.2f seconds\n", 
           packet_count, (float)(end_time - start_time) / 1000000.0);
    printf("Average processing time per packet: %.2f microseconds\n", 
           (float)(end_time - start_time) / packet_count);
    
    pcap_close(handle);
}

int main(void) {
    printf("Starting performance tests...\n");
    
    test_pcap_init_performance();
    test_packet_processing_performance();
    
    printf("\nPerformance tests completed.\n");
    return 0;
} 