#include "input_dispatch.h"
#include "input_register.h"
#include "packet_queue.h"
#include "debug_mode.h"

#include <stdio.h>

/**
 * Dispatches execution to the appropriate input handler based on CLI config.
 *
 * @param packet_queue The queue and config provided by main program.
 * @return 0 on success, -1 on failure.
 */
int dispatch_input_handler(packet_queue_t *packet_queue)
{
    const t_input_handler *handlers = get_input_handlers();
    input_type_e input_type = packet_queue->cli_config->input_type;

    if (input_type < 0 || input_type >= INPUT_TYPE_MAX || handlers[input_type].handler == NULL) {
        LOG_ERROR("Invalid or unregistered input type: %d", input_type);
        return -1;
    }

    LOG_INFO("🚚 Dispatching input handler for type => %s 🚚", get_input_type_string(input_type));
    return handlers[input_type].handler(packet_queue);
}

/**
 * Converts an input type enum to a human-readable string.
 * Useful for logging and debugging.
 */
const char *get_input_type_string(input_type_e input_type)
{
    switch (input_type) {
        case INPUT_TYPE_INTERFACE:
            return "Interface";
        case INPUT_TYPE_PCAP_FILE:
            return "PCAP File";
        default:
            return "Unknown";
    }
}