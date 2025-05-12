#ifndef TCP_TRACKER_H
#define TCP_TRACKER_H

#include <stdint.h>
#include <stdio.h>
#include "uthash.h"

typedef struct {
    char src_ip[64];
    char dst_ip[64];
    unsigned short src_port;
    unsigned short dst_port;

    uint64_t start_time_ms;
    uint64_t end_time_ms;

    uint64_t packet_count_in;
    uint64_t packet_count_out;
} tcp_connection_t;

typedef struct tcp_conn_map {
    char key[128];
    tcp_connection_t conn_data;
    UT_hash_handle hh;
} tcp_conn_map_t;

void tcp_tracker_process_packet(const char *src_ip, const char *dst_ip,
                                unsigned short src_port, unsigned short dst_port,
                                uint8_t tcp_flags);

void tcp_tracker_set_output_file(FILE *fp);
void tcp_tracker_cleanup_all(void);

#endif // TCP_TRACKER_H