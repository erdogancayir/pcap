#include "cli_config.h"
#include "input_handler.h"
#include "input_register.h"
#include "input_dispatch.h"
#include "cli_parser.h"


int main(int argc, char **argv) {
    cli_config_t config;
    if (parse_cli_args(argc, argv, &config) != 0)
        return 1;

    register_all_input_handlers();

    if (dispatch_input_handler(config.input_type, config.interface_or_file) != 0)
        return 1;

    return 0;
}