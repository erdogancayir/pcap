#include "input_handler.h"
#include "debug_mode.h"

#include <stdio.h>

// Static global table of registered input handlers
static t_input_handler g_input_handlers[INPUT_TYPE_MAX];

/**
 * Registers a handler function for a given input type.
 *
 * @param type    The input source type (e.g., interface, pcap file).
 * @param handler Function pointer to the handler implementation.
 * @return 0 on success, -1 on failure.
 */
int register_input_handler(input_type_e type, int (*handler)(packet_queue_t *))
{
    if (type < 0 || type >= INPUT_TYPE_MAX || handler == NULL)
        return -1;

    g_input_handlers[type].handler = handler;
    return 0;
}

/**
 * Registers all supported input handlers.
 * This should be called during application initialization.
 */
void register_all_input_handlers(void)
{
    register_input_handler(INPUT_TYPE_INTERFACE, input_handle_interface);
    register_input_handler(INPUT_TYPE_PCAP_FILE, input_handle_pcap_file);

    LOG_DEBUG("Registered all input type handlers.");
}

/**
 * Returns the table of registered input handlers.
 * Used by the dispatcher to route execution.
 */
const t_input_handler *get_input_handlers(void)
{
    return g_input_handlers;
}