#include <pcap.h>
#include "packet_queue.h"
#include "debug_mode.h"
#include "packet_handler.h"

void *sniffer_thread(void *arg)
{
    packet_queue_t *packet_queue = (packet_queue_t *)arg;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;

    cli_config_t *cli_config = packet_queue->cli_config;

    handle = pcap_open_live(cli_config->interface_or_file, BUFSIZ, 1, 100, errbuf);
    if (!handle) {
        fprintf(stderr, "pcap_open_live() failed: %s\n", errbuf);
        pthread_exit(NULL);
    }

    struct bpf_program fp;
    char *filter_exp = "tcp or udp";

    if (pcap_compile(handle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "pcap_compile() failed: %s\n", pcap_geterr(handle));
        pcap_close(handle);
        pthread_exit(NULL);
    }

    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "pcap_setfilter() failed: %s\n", pcap_geterr(handle));
        pcap_freecode(&fp);
        pcap_close(handle);
        pthread_exit(NULL);
    }

    pcap_freecode(&fp);

    pcap_loop(handle, 0, packet_handler, (u_char *)packet_queue);

    pcap_close(handle);
    return NULL;
}