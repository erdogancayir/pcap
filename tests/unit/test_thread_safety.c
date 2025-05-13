#include "../test_framework.h"
#include "../../include/packet_queue.h"
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define ITERATIONS 1000

// Test data structure
typedef struct {
    int value;
    char data[100];
} TestPacket;

// Thread arguments
typedef struct {
    PacketQueue *queue;
    int thread_id;
    ThreadSafeCounter *counter;
} ThreadArgs;

void *producer_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    TestPacket packet;
    
    for (int i = 0; i < ITERATIONS; i++) {
        packet.value = args->thread_id * ITERATIONS + i;
        snprintf(packet.data, sizeof(packet.data), "Thread %d, Packet %d", 
                args->thread_id, i);
        
        // Try to enqueue the packet
        if (packet_queue_enqueue(args->queue, &packet, sizeof(packet)) == 0) {
            increment_counter(args->counter);
        }
        
        // Simulate some work
        usleep(100);
    }
    return NULL;
}

void *consumer_thread(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    TestPacket packet;
    size_t len;
    
    for (int i = 0; i < ITERATIONS; i++) {
        // Try to dequeue a packet
        if (packet_queue_dequeue(args->queue, &packet, &len) == 0) {
            increment_counter(args->counter);
            
            // Verify packet data
            TEST_ASSERT(len == sizeof(packet), "Dequeued packet size should match");
            TEST_ASSERT(packet.value >= 0, "Packet value should be valid");
            TEST_ASSERT(strlen(packet.data) > 0, "Packet data should not be empty");
        }
        
        // Simulate some work
        usleep(100);
    }
    return NULL;
}

void test_queue_thread_safety(void) {
    PacketQueue queue;
    ThreadSafeCounter counter;
    pthread_t producers[NUM_THREADS];
    pthread_t consumers[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];
    
    // Initialize queue and counter
    TEST_ASSERT(packet_queue_init(&queue, 1000) == 0, "Queue initialization should succeed");
    init_thread_safe_counter(&counter);
    
    // Create producer threads
    for (int i = 0; i < NUM_THREADS; i++) {
        args[i].queue = &queue;
        args[i].thread_id = i;
        args[i].counter = &counter;
        pthread_create(&producers[i], NULL, producer_thread, &args[i]);
    }
    
    // Create consumer threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&consumers[i], NULL, consumer_thread, &args[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(producers[i], NULL);
        pthread_join(consumers[i], NULL);
    }
    
    // Verify results
    int total_operations = get_counter_value(&counter);
    TEST_ASSERT(total_operations > 0, "Threads should have performed operations");
    TEST_ASSERT(total_operations <= NUM_THREADS * ITERATIONS * 2, 
                "Total operations should not exceed maximum possible");
    
    // Cleanup
    packet_queue_destroy(&queue);
}

void test_queue_overflow(void) {
    PacketQueue queue;
    TestPacket packet;
    
    // Initialize small queue
    TEST_ASSERT(packet_queue_init(&queue, 5) == 0, "Queue initialization should succeed");
    
    // Fill queue
    for (int i = 0; i < 5; i++) {
        packet.value = i;
        TEST_ASSERT(packet_queue_enqueue(&queue, &packet, sizeof(packet)) == 0,
                    "Queue should accept packets until full");
    }
    
    // Try to overflow
    TEST_ASSERT(packet_queue_enqueue(&queue, &packet, sizeof(packet)) != 0,
                "Queue should reject packets when full");
    
    // Cleanup
    packet_queue_destroy(&queue);
}

void test_queue_underflow(void) {
    PacketQueue queue;
    TestPacket packet;
    size_t len;
    
    // Initialize empty queue
    TEST_ASSERT(packet_queue_init(&queue, 5) == 0, "Queue initialization should succeed");
    
    // Try to dequeue from empty queue
    TEST_ASSERT(packet_queue_dequeue(&queue, &packet, &len) != 0,
                "Queue should reject dequeue when empty");
    
    // Cleanup
    packet_queue_destroy(&queue);
}

int main(void) {
    init_test_suite();
    
    printf("Running thread safety tests...\n");
    test_queue_thread_safety();
    test_queue_overflow();
    test_queue_underflow();
    
    print_test_summary();
    return test_stats.failed_tests > 0 ? 1 : 0;
} 