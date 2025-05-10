#include "packet_queue.h"

void packet_queue_init(packet_queue_t *q)
{
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);
}

void packet_queue_destroy(packet_queue_t *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);
}

void packet_queue_push(packet_queue_t *q, captured_packet_t *packet)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == PACKET_QUEUE_SIZE) {
        pthread_cond_wait(&q->not_full, &q->mutex);
    }

    q->buffer[q->tail] = packet;
    q->tail = (q->tail + 1) % PACKET_QUEUE_SIZE;
    q->count++;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock(&q->mutex);
}

captured_packet_t *packet_queue_pop(packet_queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
    }

    captured_packet_t *packet = q->buffer[q->head];
    q->head = (q->head + 1) % PACKET_QUEUE_SIZE;
    q->count--;

    pthread_cond_signal(&q->not_full);
    pthread_mutex_unlock(&q->mutex);

    return packet;
}