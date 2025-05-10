#include "cli_config.h"
#include "input_handler.h"
#include <string.h>
#include <stdio.h>
#include "my_libc.h"

int parse_cli_args(int argc, char **argv, cli_config_t *config) {
    if (argc < 5) {
        print_usage(argv[0]);
        return -1;
    }

    my_memset(config, 0, sizeof(cli_config_t));
    config->input_type = INPUT_TYPE_MAX;

    for (int i = 1; i < argc; i++) {
        if (my_strcmp(argv[i], "-i") == 0 && (i + 1) < argc) {
            config->input_type = INPUT_TYPE_INTERFACE;
            strncpy(config->interface_or_file, argv[i + 1], sizeof(config->interface_or_file) - 1);
            i++;
        } else if (my_strcmp(argv[i], "-r") == 0 && (i + 1) < argc) {
            config->input_type = INPUT_TYPE_PCAP_FILE;
            strncpy(config->interface_or_file, argv[i + 1], sizeof(config->interface_or_file) - 1);
            i++;
        } else if (my_strcmp(argv[i], "-o") == 0 && (i + 1) < argc) {
            strncpy(config->output_file, argv[i + 1], sizeof(config->output_file) - 1);
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