#include "packet_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

/**
 * Packet handler callback for libpcap.
 * This function is invoked by pcap_loop() for each captured packet.
 * It parses the IP header and delegates processing to protocol-specific handlers.
 *
 * @param args    Pointer to packet_queue_t, passed via pcap_loop().
 * @param header  Metadata about the captured packet (timestamp, length, etc.).
 * @param packet  Raw packet data including Ethernet and IP headers.
 */
void packet_handler(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
    packet_queue_t *queue = (packet_queue_t *)args;

    // Parse the IP header (assumes Ethernet header is ETHERNET_HDR_LEN bytes long)
    const struct ip *ip_header = (const struct ip *)(packet + ETHERNET_HDR_LEN);

    if (ip_header == NULL)
        return; // Invalid or corrupted packet

    uint8_t protocol = ip_header->ip_p;
    int ip_header_len = ip_header->ip_hl * 4; // ip_hl is in 32-bit words

    // Dispatch to protocol-specific handler
    switch (protocol) {
        case IPPROTO_TCP:
            handle_tcp_packet(packet, header, queue, ip_header, ip_header_len);
            break;
        case IPPROTO_UDP:
            handle_udp_packet(packet, queue, ip_header);
            break;
        default:
            //DEBUG("[INFO] Unknown protocol: %s", protocol_to_string(protocol));
            break;
    }
}

const char *protocol_to_string(uint8_t proto)
{
    switch (proto) {
        case IPPROTO_TCP:  return "TCP";
        case IPPROTO_UDP:  return "UDP";
        case IPPROTO_ICMP: return "ICMP";
        case IPPROTO_IGMP: return "IGMP";
        case IPPROTO_IP:   return "IP";
        default:           return "UNKNOWN";
    }
}