#include "packet_queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include "debug_mode.h"
#include <netinet/udp.h>

void *packet_writer_thread(void *arg)
{
    packet_queue_t *queue = (packet_queue_t *)arg;
    cli_config_t *config = queue->cli_config;

    return NULL;
}   