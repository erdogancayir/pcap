#include "cli_config.h"
#include "input_handler.h"
#include "input_register.h"
#include "input_dispatch.h"
#include "cli_parser.h"

#include "packet_queue.h"
#include "debug_mode.h"


int main(int argc, char **argv)
{
    cli_config_t config;
    if (parse_cli_args(argc, argv, &config) != 0)
        return 1;

    packet_queue_t queue;
    packet_queue_init(&queue);
    queue.cli_config = &config;
    
    register_all_input_handlers();

    if (dispatch_input_handler(&queue) != 0)
        return 1;

    return 0;
}