#ifndef CAPTURED_PACKET_H
#define CAPTURED_PACKET_H

typedef struct {
    char src_mac[18];
    char dst_mac[18];

    char src_ip[64];
    char dst_ip[64];

    unsigned short src_port;
    unsigned short dst_port;

    int is_http;
    char host[256];
    char user_agent[512];
} captured_packet_t;


#endif