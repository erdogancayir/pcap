#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "cli_config.h"
#include "packet_queue.h"

typedef struct s_input_handler {
    int (*handler)(packet_queue_t *packet_queue);
} t_input_handler;

int input_handle_interface(packet_queue_t *packet_queue);
int input_handle_pcap_file(packet_queue_t *packet_queue);

void *sniffer_thread(void *arg);
void *packet_writer_thread(void *arg);

#endif