#include "cli_config.h"
#include "input_handler.h"
#include "input_register.h"
#include "input_dispatch.h"
#include "cli_parser.h"

#include "packet_queue.h"
#include "debug_mode.h"


int main(int argc, char **argv)
{
    DEBUG("[INFO] Starting packet capture application...");
    cli_config_t config;
    if (parse_cli_args(argc, argv, &config) != 0)
        return 1;

    packet_queue_t queue;
    packet_queue_init(&queue);
    queue.cli_config = &config;

    // log queu and config
    DEBUG("[INFO] Queue initialized with size: %d\n", PACKET_QUEUE_SIZE);
    DEBUG("[INFO] CLI Config: input_type=%d, interface_or_file=%s, output_file=%s",
           config.input_type, config.interface_or_file, config.output_file);
    
    DEBUG("[INFO] Registering input handlers...\n");
    
    register_all_input_handlers();

    if (dispatch_input_handler(&queue) != 0)
        return 1;

    DEBUG("[INFO] Input handler dispatched successfully.");
    return 0;
}