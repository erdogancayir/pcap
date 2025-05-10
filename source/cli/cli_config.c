#include "cli_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char *progname) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s -i <interface> -o <output_file>\n", progname);
    fprintf(stderr, "  %s -r <pcap_file> -o <output_file>\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i\tRead from network interface\n");
    fprintf(stderr, "  -r\tRead from pcap file\n");
    fprintf(stderr, "  -o\tOutput file to write results\n");
}