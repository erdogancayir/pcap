#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include "captured_packet.h"
#include "cli_config.h"

#include <pthread.h>

#define PACKET_QUEUE_SIZE 4096

typedef struct {
    captured_packet_t *buffer;
    int head;
    int tail;
    int count;
    int done;

    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;

    const cli_config_t *cli_config;
} packet_queue_t;

packet_queue_t *packet_queue_create(const cli_config_t *cli_config);

void packet_queue_destroy(packet_queue_t *q);
void packet_queue_enqueue(packet_queue_t *q, const captured_packet_t *packet);
int  packet_queue_dequeue(packet_queue_t *q, captured_packet_t *out_packet);

void packet_queue_mark_done(packet_queue_t *q);

#endif