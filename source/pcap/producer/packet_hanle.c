#include "packet_queue.h"
#include "debug_mode.h"

void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    packet_queue_t *queue = (packet_queue_t *)args;

    captured_packet_t *pkt = malloc(sizeof(captured_packet_t));
    if (!pkt)
        return;

    pkt->timestamp = header->ts;
    pkt->original_length = header->len;
    pkt->captured_length = header->caplen;

    pkt->packet_data = malloc(header->caplen);
    if (!pkt->packet_data) {
        free(pkt);
        return;
    }

    memcpy(pkt->packet_data, packet, header->caplen);

    DEBUG("[INFO] Packet captured: length=%u, caplen=%u", header->len, header->caplen);
    packet_queue_push(queue, pkt);
}