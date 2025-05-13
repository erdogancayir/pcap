#include "../test_framework.h"
#include "../../include/packet_queue.h"
#include "../../include/cli_config.h"
#include "../../include/packet_handler.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_OPERATIONS 1000
#define TOTAL_PACKETS (NUM_THREADS * NUM_OPERATIONS)

typedef struct {
    packet_queue_t *queue;
    int thread_id;
    int *active_producers;
    int *packets_processed;
    pthread_mutex_t *producer_mutex;
    pthread_mutex_t *stats_mutex;
} ThreadArgs;

void *producer_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    captured_packet_t packet;
    int i;
    
    for (i = 0; i < NUM_OPERATIONS; i++) {
        // Create a test packet
        snprintf(packet.src_mac, sizeof(packet.src_mac), "00:11:22:33:44:55");
        snprintf(packet.dst_mac, sizeof(packet.dst_mac), "66:77:88:99:AA:BB");
        snprintf(packet.src_ip, sizeof(packet.src_ip), "192.168.1.%d", i % 255);
        snprintf(packet.dst_ip, sizeof(packet.dst_ip), "10.0.0.%d", i % 255);
        packet.src_port = 12345 + i;
        packet.dst_port = 80;
        packet.is_http = 1;
        snprintf(packet.host, sizeof(packet.host), "example.com");
        snprintf(packet.user_agent, sizeof(packet.user_agent), "TestClient/1.0");
        
        packet_queue_enqueue(args->queue, &packet);
        usleep(100); // Small delay to simulate real-world conditions
    }
    
    // Decrement active producers count
    pthread_mutex_lock(args->producer_mutex);
    (*args->active_producers)--;
    if (*args->active_producers == 0) {
        packet_queue_mark_done(args->queue);
    }
    pthread_mutex_unlock(args->producer_mutex);
    
    return NULL;
}

void *consumer_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    captured_packet_t packet;
    int local_count = 0;
    
    while (1) {
        if (packet_queue_dequeue(args->queue, &packet) == 0) {
            // Verify packet contents
            TEST_ASSERT(strlen(packet.src_mac) > 0, "Source MAC should be set");
            TEST_ASSERT(strlen(packet.dst_mac) > 0, "Destination MAC should be set");
            TEST_ASSERT(strlen(packet.src_ip) > 0, "Source IP should be set");
            TEST_ASSERT(strlen(packet.dst_ip) > 0, "Destination IP should be set");
            TEST_ASSERT(packet.src_port > 0, "Source port should be set");
            TEST_ASSERT(packet.dst_port > 0, "Destination port should be set");
            local_count++;
        } else {
            // If dequeue failed and queue is done, break the loop
            if (args->queue->done) {
                break;
            }
            fprintf(stderr, "Thread %d: Failed to dequeue packet\n", args->thread_id);
        }
        usleep(100); // Small delay to simulate real-world conditions
    }
    
    // Update total packets processed
    pthread_mutex_lock(args->stats_mutex);
    *args->packets_processed += local_count;
    pthread_mutex_unlock(args->stats_mutex);
    
    printf("Consumer thread %d processed %d packets\n", args->thread_id, local_count);
    return NULL;
}

void test_concurrent_access(void) {
    pthread_t threads[NUM_THREADS * 2];
    ThreadArgs args[NUM_THREADS * 2];
    cli_config_t config = {
        .input_type = INPUT_TYPE_INTERFACE,
        .interface_or_file = "lo",
        .output_file = NULL
    };
    packet_queue_t *queue;
    int i;
    int active_producers = NUM_THREADS;
    int total_packets_processed = 0;
    pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
    
    // Initialize queue
    queue = packet_queue_create(&config);
    TEST_ASSERT(queue != NULL, "Queue creation should succeed");
    
    // Create producer and consumer threads
    for (i = 0; i < NUM_THREADS; i++) {
        args[i].queue = queue;
        args[i].thread_id = i;
        args[i].active_producers = &active_producers;
        args[i].packets_processed = &total_packets_processed;
        args[i].producer_mutex = &producer_mutex;
        args[i].stats_mutex = &stats_mutex;
        pthread_create(&threads[i], NULL, producer_thread, &args[i]);
        
        args[i + NUM_THREADS].queue = queue;
        args[i + NUM_THREADS].thread_id = i;
        args[i + NUM_THREADS].active_producers = &active_producers;
        args[i + NUM_THREADS].packets_processed = &total_packets_processed;
        args[i + NUM_THREADS].producer_mutex = &producer_mutex;
        args[i + NUM_THREADS].stats_mutex = &stats_mutex;
        pthread_create(&threads[i + NUM_THREADS], NULL, consumer_thread, &args[i + NUM_THREADS]);
    }
    
    // Wait for all threads to complete
    for (i = 0; i < NUM_THREADS * 2; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Verify total packets processed
    TEST_ASSERT(total_packets_processed == TOTAL_PACKETS,
                "Total packets processed should match expected count");
    printf("Total packets processed: %d (expected: %d)\n", 
           total_packets_processed, TOTAL_PACKETS);
    
    // Cleanup
    pthread_mutex_destroy(&producer_mutex);
    pthread_mutex_destroy(&stats_mutex);
    packet_queue_destroy(queue);
}

void test_queue_full(void) {
    cli_config_t config = {
        .input_type = INPUT_TYPE_INTERFACE,
        .interface_or_file = "lo",
        .output_file = NULL
    };
    packet_queue_t *queue;
    captured_packet_t packet;
    int i;
    
    // Initialize queue
    queue = packet_queue_create(&config);
    TEST_ASSERT(queue != NULL, "Queue creation should succeed");
    
    // Fill queue with test packets
    for (i = 0; i < PACKET_QUEUE_SIZE; i++) {
        snprintf(packet.src_mac, sizeof(packet.src_mac), "00:11:22:33:44:55");
        snprintf(packet.dst_mac, sizeof(packet.dst_mac), "66:77:88:99:AA:BB");
        snprintf(packet.src_ip, sizeof(packet.src_ip), "192.168.1.%d", i % 255);
        snprintf(packet.dst_ip, sizeof(packet.dst_ip), "10.0.0.%d", i % 255);
        packet.src_port = 12345 + i;
        packet.dst_port = 80;
        packet.is_http = 1;
        snprintf(packet.host, sizeof(packet.host), "example.com");
        snprintf(packet.user_agent, sizeof(packet.user_agent), "TestClient/1.0");
        
        packet_queue_enqueue(queue, &packet);
    }
    
    // Try to add one more packet
    snprintf(packet.src_mac, sizeof(packet.src_mac), "00:11:22:33:44:55");
    snprintf(packet.dst_mac, sizeof(packet.dst_mac), "66:77:88:99:AA:BB");
    snprintf(packet.src_ip, sizeof(packet.src_ip), "192.168.1.100");
    snprintf(packet.dst_ip, sizeof(packet.dst_ip), "10.0.0.100");
    packet.src_port = 12345;
    packet.dst_port = 80;
    packet.is_http = 1;
    snprintf(packet.host, sizeof(packet.host), "example.com");
    snprintf(packet.user_agent, sizeof(packet.user_agent), "TestClient/1.0");
    
    packet_queue_enqueue(queue, &packet); // Should not block or fail
    
    // Cleanup
    packet_queue_destroy(queue);
}

void test_queue_empty(void) {
    cli_config_t config = {
        .input_type = INPUT_TYPE_INTERFACE,
        .interface_or_file = "lo",
        .output_file = NULL
    };
    packet_queue_t *queue;
    captured_packet_t packet;
    
    // Initialize queue
    queue = packet_queue_create(&config);
    TEST_ASSERT(queue != NULL, "Queue creation should succeed");
    
    // Try to dequeue from empty queue
    TEST_ASSERT(packet_queue_dequeue(queue, &packet) != 0,
                "Queue should reject dequeue when empty");
    
    // Cleanup
    packet_queue_destroy(queue);
}

int main(void) {
    init_test_suite();
    
    printf("Running thread safety tests...\n");
    test_concurrent_access();
    test_queue_full();
    test_queue_empty();
    
    print_test_summary();
    return test_stats.failed_tests > 0 ? 1 : 0;
} 