#include "cli_config.h"
#include "my_libc.h"

#include <stdio.h>
#include <string.h>

int parse_cli_args(int argc, char **argv, cli_config_t *config)
{
    if (argc < 5) {
        print_usage(argv[0]);
        return -1;
    }

    my_memset(config, 0, sizeof(cli_config_t));
    config->input_type = INPUT_TYPE_MAX;

    for (int i = 1; i < argc; i++) {
        if (my_strcmp(argv[i], "-i") == 0 && (i + 1) < argc) {
            config->input_type = INPUT_TYPE_INTERFACE;
            config->interface_or_file = strdup(argv[i + 1]);
            i++;
        } else if (my_strcmp(argv[i], "-r") == 0 && (i + 1) < argc) {
            config->input_type = INPUT_TYPE_PCAP_FILE;
            config->interface_or_file = strdup(argv[i + 1]);
            i++;
        } else if (my_strcmp(argv[i], "-o") == 0 && (i + 1) < argc) {
            config->output_file = strdup(argv[i + 1]);
            i++;
        } else {
            fprintf(stderr, "Unknown or incomplete argument: %s\n", argv[i]);
            print_usage(argv[0]);
            return -1;
        }
    }

    if (config->input_type == INPUT_TYPE_MAX || config->interface_or_file[0] == '\0' || config->output_file[0] == '\0') {
        fprintf(stderr, "Error: Missing required arguments.\n");
        print_usage(argv[0]);
        return -1;
    }

    return 0;
}

void print_usage(const char *progname)
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s -i <interface> -o <output_file>\n", progname);
    fprintf(stderr, "  %s -r <pcap_file> -o <output_file>\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -i\tRead from network interface\n");
    fprintf(stderr, "  -r\tRead from pcap file\n");
    fprintf(stderr, "  -o\tOutput file to write results\n");
}

void free_config(cli_config_t *config)
{
    if (config->interface_or_file) {
        free(config->interface_or_file);
        config->interface_or_file = NULL;
    }
    if (config->output_file) {
        free(config->output_file);
        config->output_file = NULL;
    }
}