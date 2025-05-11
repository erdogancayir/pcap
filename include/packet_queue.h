#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <pthread.h>
#include "cli_config.h"
#include "captured_packet.h"

#define PACKET_QUEUE_SIZE 4096

typedef struct {
    captured_packet_t buffer[PACKET_QUEUE_SIZE];
    int head;
    int tail;
    int count;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

    cli_config_t *cli_config;
} packet_queue_t;

void packet_queue_init(packet_queue_t *q);
void packet_queue_destroy(packet_queue_t *q);
void packet_queue_enqueue(packet_queue_t *q, const captured_packet_t *packet);
int  packet_queue_dequeue(packet_queue_t *q, captured_packet_t *out_packet);

#endif // PACKET_QUEUE_H