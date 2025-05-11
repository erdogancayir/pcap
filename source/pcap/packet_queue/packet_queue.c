#include "packet_queue.h"
#include <string.h>

void packet_queue_init(packet_queue_t *q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

void packet_queue_destroy(packet_queue_t *q)
{
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

void packet_queue_enqueue(packet_queue_t *q, const captured_packet_t *packet)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == PACKET_QUEUE_SIZE) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    q->buffer[q->tail] = *packet;  // copy by value
    q->tail = (q->tail + 1) % PACKET_QUEUE_SIZE;
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

int packet_queue_dequeue(packet_queue_t *q, captured_packet_t *out_packet)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    *out_packet = q->buffer[q->head];  // copy by value
    q->head = (q->head + 1) % PACKET_QUEUE_SIZE;
    q->count--;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);

    return 0;
}