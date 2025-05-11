#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include <pcap/pcap.h>

typedef enum e_input_type
{
    INPUT_TYPE_INTERFACE,
    INPUT_TYPE_PCAP_FILE,
    INPUT_TYPE_MAX
} input_type_e;

typedef struct s_cli_config
{
    input_type_e input_type;
    char *interface_or_file;
    char *output_file;
} cli_config_t;

void print_usage(const char *progname);
void free_config(cli_config_t *config);

#endif