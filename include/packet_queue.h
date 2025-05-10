#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include "cli_config.h"


#define PACKET_QUEUE_SIZE 4096

typedef struct {
    struct timeval timestamp;
    uint32_t original_length;
    uint32_t captured_length;
    uint8_t *packet_data;
} captured_packet_t;

typedef struct {
    captured_packet_t *buffer[PACKET_QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    
    cli_config_t *cli_config;
} packet_queue_t;

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet);

void packet_queue_init(packet_queue_t *q);
void packet_queue_destroy(packet_queue_t *q);
void packet_queue_push(packet_queue_t *q, captured_packet_t *packet);
captured_packet_t *packet_queue_pop(packet_queue_t *q);

#endif
