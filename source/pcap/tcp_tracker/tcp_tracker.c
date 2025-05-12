#include <netinet/tcp.h>
#include <time.h>

#include "tcp_tracker.h"
#include "my_libc.h"
#include "uthash.h"

static tcp_conn_map_t *tcp_table = NULL;
static FILE *output_fp = NULL;

static uint64_t get_current_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void tcp_tracker_set_output_file(FILE *fp)
{
    output_fp = fp;
}

void tcp_tracker_process_packet(const char *src_ip, const char *dst_ip,
                                unsigned short src_port, unsigned short dst_port,
                                uint8_t tcp_flags)
{
    char key[128];
    snprintf(key, sizeof(key), "%s:%u-%s:%u", src_ip, src_port, dst_ip, dst_port);

    tcp_conn_map_t *entry = NULL;
    HASH_FIND_STR(tcp_table, key, entry);

    if (!entry) {
        entry = malloc(sizeof(tcp_conn_map_t));
        if (!entry) return;

        memset(entry, 0, sizeof(*entry));
        strcpy(entry->key, key);

        strcpy(entry->conn_data.src_ip, src_ip);
        strcpy(entry->conn_data.dst_ip, dst_ip);
        entry->conn_data.src_port = src_port;
        entry->conn_data.dst_port = dst_port;
        entry->conn_data.start_time_ms = get_current_time_ms();

        HASH_ADD_STR(tcp_table, key, entry);
    }

    if (my_strcmp(src_ip, entry->conn_data.src_ip) == 0 && src_port == entry->conn_data.src_port)
        entry->conn_data.packet_count_out++;
    else
        entry->conn_data.packet_count_in++;

    if (tcp_flags & (TH_FIN | TH_RST)) {
        entry->conn_data.end_time_ms = get_current_time_ms();
        uint64_t duration = entry->conn_data.end_time_ms - entry->conn_data.start_time_ms;

        FILE *out = output_fp ? output_fp : stdout;
        fprintf(out, "\n[TCP CLOSED] %s:%u -> %s:%u\n",
                entry->conn_data.src_ip, entry->conn_data.src_port,
                entry->conn_data.dst_ip, entry->conn_data.dst_port);

        fprintf(out, "IN :  %lu packets\n", entry->conn_data.packet_count_in);
        fprintf(out, "OUT:  %lu packets\n", entry->conn_data.packet_count_out);
        fprintf(out, "Duration: %lu ms\n", duration);
        fprintf(out, "---------------------------------------\n");
        fflush(out);

        HASH_DEL(tcp_table, entry);
        free(entry);
    }
}

void tcp_tracker_cleanup_all(void)
{
    tcp_conn_map_t *entry, *tmp;
    HASH_ITER(hh, tcp_table, entry, tmp) {
        HASH_DEL(tcp_table, entry);
        free(entry);
    }
}