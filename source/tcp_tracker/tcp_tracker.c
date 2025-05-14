#include <netinet/tcp.h>
#include <time.h>

#include "tcp_tracker.h"
#include "my_libc.h"
#include "uthash.h"

// Global hash table for tracking active TCP connections
static tcp_conn_map_t *tcp_table = NULL;

// Output file handle used to log closed connections
static FILE *output_fp = NULL;

/**
 * Gets the current time in nanoseconds using a monotonic clock.
 * This is used to track the start and end times of TCP connections.
 */
static uint64_t get_current_time_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000000000ULL + ts.tv_nsec);
}

/**
 * Converts nanoseconds to milliseconds, ensuring at least 1ms for very short durations
 */
static uint64_t ns_to_ms(uint64_t ns)
{
    uint64_t ms = ns / 1000000ULL;
    return ms > 0 ? ms : 1;  // Ensure minimum 1ms duration
}

/**
 * Sets the output file for TCP connection summaries.
 * If not set, logs will default to stdout.
 *
 * @param fp File pointer to write TCP connection summaries.
 */
void tcp_tracker_set_output_file(FILE *fp)
{
    output_fp = fp;
}

/**
 * Tracks and updates the state of a TCP connection.
 * Handles packet direction, connection keying, and closure on FIN/RST flags.
 *
 * @param src_ip Source IP address.
 * @param dst_ip Destination IP address.
 * @param src_port Source TCP port.
 * @param dst_port Destination TCP port.
 * @param tcp_flags TCP header flags (e.g., SYN, FIN, RST).
 */
void tcp_tracker_process_packet(const char *src_ip, const char *dst_ip,
                                unsigned short src_port, unsigned short dst_port,
                                uint8_t tcp_flags)
{
    char key[128];

    // Normalize key for direction-independent connection tracking
    int is_a_before_b = my_strcmp(src_ip, dst_ip) < 0 ||
                        (my_strcmp(src_ip, dst_ip) == 0 && src_port < dst_port);

    if (is_a_before_b)
        snprintf(key, sizeof(key), "%s:%u-%s:%u", src_ip, src_port, dst_ip, dst_port);
    else
        snprintf(key, sizeof(key), "%s:%u-%s:%u", dst_ip, dst_port, src_ip, src_port);

    // Try to find the connection in the hash table
    tcp_conn_map_t *entry = NULL;
    HASH_FIND_STR(tcp_table, key, entry);

    // New connection if not found
    if (!entry) {
        entry = malloc(sizeof(tcp_conn_map_t));
        if (!entry) return;

        memset(entry, 0, sizeof(*entry));
        strcpy(entry->key, key);

        // Store direction (always save src/dst as A->B based on key)
        if (is_a_before_b) {
            strcpy(entry->conn_data.src_ip, src_ip);
            strcpy(entry->conn_data.dst_ip, dst_ip);
            entry->conn_data.src_port = src_port;
            entry->conn_data.dst_port = dst_port;
        } else {
            strcpy(entry->conn_data.src_ip, dst_ip);
            strcpy(entry->conn_data.dst_ip, src_ip);
            entry->conn_data.src_port = dst_port;
            entry->conn_data.dst_port = src_port;
        }

        entry->conn_data.start_time_ns = get_current_time_ns();
        HASH_ADD_STR(tcp_table, key, entry);
    }

    // Determine packet direction to count IN/OUT
    int is_fwd = my_strcmp(src_ip, entry->conn_data.src_ip) == 0 &&
                 src_port == entry->conn_data.src_port &&
                 my_strcmp(dst_ip, entry->conn_data.dst_ip) == 0 &&
                 dst_port == entry->conn_data.dst_port;

    if (is_fwd)
        entry->conn_data.packet_count_out++;
    else
        entry->conn_data.packet_count_in++;

    // Close the connection if FIN or RST is seen
    if (tcp_flags & (TH_FIN | TH_RST))
    {
        if (entry->conn_data.end_time_ns == 0) 
        {
            entry->conn_data.end_time_ns = get_current_time_ns();
            uint64_t duration_ns = entry->conn_data.end_time_ns - entry->conn_data.start_time_ns;
            uint64_t duration_ms = ns_to_ms(duration_ns);

           FILE *out = output_fp ? output_fp : stdout;
            fprintf(out, "\n[TCP CLOSED] %s:%u -> %s:%u\n",
                    entry->conn_data.src_ip, entry->conn_data.src_port,
                    entry->conn_data.dst_ip, entry->conn_data.dst_port);

            fprintf(out, "IN :  %llu packets\n", entry->conn_data.packet_count_in);
            fprintf(out, "OUT:  %llu packets\n", entry->conn_data.packet_count_out);
            fprintf(out, "Duration: %llu ms\n", duration_ms);
            fprintf(out, "---------------------------------------\n");
            fflush(out);

            // Remove and free entry
            HASH_DEL(tcp_table, entry);
            free(entry);
        }
    }
}

/**
 * Frees all remaining connections from the tracking table.
 * Useful for cleanup at the end of the program or when resetting state.
 */
void tcp_tracker_cleanup_all(void)
{
    tcp_conn_map_t *entry, *tmp;
    HASH_ITER(hh, tcp_table, entry, tmp) {
        HASH_DEL(tcp_table, entry);
        free(entry);
    }
}