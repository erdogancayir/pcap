#include "packet_queue.h"
#include <stdlib.h>

#include "debug_mode.h"

packet_queue_t *packet_queue_create(const cli_config_t *cli_config)
{
    packet_queue_t *q = malloc(sizeof(packet_queue_t));
    if (!q) return NULL;

    q->buffer = malloc(sizeof(captured_packet_t) * PACKET_QUEUE_SIZE);
    if (!q->buffer) {
        free(q);
        return NULL;
    }

    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->done = 0;
    q->cli_config = cli_config;

    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);

    return q;
}

void packet_queue_destroy(packet_queue_t *q)
{
    if (!q) return;
    
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);

    free(q->buffer);
    free(q);
}

void packet_queue_enqueue(packet_queue_t *q, const captured_packet_t *packet)
{
    pthread_mutex_lock(&q->mutex);
    // DEBUG("\033[1;31m[LOCK] MUTEX locked in enqueue\033[0m");

    while (q->count == PACKET_QUEUE_SIZE && !q->done) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    // DEBUG("[INFO] Enqueueing packet");
    
    if (q->done) {
        pthread_mutex_unlock(&q->mutex);
        return;
    }

    q->buffer[q->tail] = *packet;  // copy by value
    q->tail = (q->tail + 1) % PACKET_QUEUE_SIZE;
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
    
    // DEBUG("\033[1;32m[UNLOCK] MUTEX unlocked in enqueue\033[0m");
}

int packet_queue_dequeue(packet_queue_t *q, captured_packet_t *out_packet)
{
    pthread_mutex_lock(&q->mutex);
    // DEBUG("\033[1;31m[LOCK] MUTEX locked in dequeue\033[0m");

    while (q->count == 0 && !q->done)
    {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    if (q->count == 0 && q->done)
    {
        pthread_mutex_unlock(&q->mutex);
        return -1;
    }

    *out_packet = q->buffer[q->head];  // copy by value
    q->head = (q->head + 1) % PACKET_QUEUE_SIZE;
    q->count--;
    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);

    // DEBUG("\033[1;32m[UNLOCK] MUTEX unlocked in dequeue\033[0m");
    return 0;
}

void packet_queue_mark_done(packet_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    q->done = 1;

    pthread_cond_broadcast(&q->not_empty);
    pthread_cond_broadcast(&q->not_full);

    pthread_mutex_unlock(&q->mutex);

    //DEBUG("[INFO] Queue marked as done.");
}