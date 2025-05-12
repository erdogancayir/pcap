#include "packet_handler.h"
#include "signal_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <pcap.h>

void *sniffer_thread(void *arg)
{
    packet_queue_t *queue = (packet_queue_t *)arg;
    char errbuf[PCAP_ERRBUF_SIZE];

    pcap_t *handle = pcap_open_live(queue->cli_config->interface_or_file, BUFSIZ, 1, 100, errbuf);
    if (!handle) {
        fprintf(stderr, "pcap_open_live() failed: %s\n", errbuf);
        pthread_exit(NULL);
    }

    set_pcap_handle(handle);

    struct bpf_program fp;
        if (pcap_compile(handle, &fp, "tcp or udp", 0, PCAP_NETMASK_UNKNOWN) == -1) {
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

    pcap_loop(handle, 0, packet_handler, (u_char *)queue);

    pcap_close(handle);

    packet_queue_mark_done(queue);

    LOG_INFO("Sniffer thread finished.");
    return NULL;
}