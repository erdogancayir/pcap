#include "input_register.h"
#include "input_dispatch.h"
#include "signal_handler.h"
#include "input_handler.h"
#include "packet_queue.h"
#include "debug_mode.h"
#include "cli_config.h"
#include "cli_parser.h"

int main(int argc, char **argv)
{
    if (argc < 5) {
        print_usage(argv[0]);
        return -1;
    }

    signal(SIGINT, sigint_handler);  // CTRL+C

    cli_config_t config = {0};
    if (parse_cli_args(argc, argv, &config) != 0) {
        return 1;
    }
    print_config_summary(&config);

    packet_queue_t *queue = packet_queue_create(&config);
    if (!queue) {
        LOG_ERROR("Failed to allocate packet queue.");
        return 1;
    }
    
    // Register input handler implementations (e.g., live interface, PCAP file)
    register_all_input_handlers();

    // Dispatch based on selected input type
    if (dispatch_input_handler(queue) != 0)
        return 1;

    // Cleanup resources
    packet_queue_destroy(queue);
    free_config(&config);

    LOG_INFO("Program exited cleanly.");
    return 0;
}