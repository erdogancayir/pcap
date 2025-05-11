#include "input_dispatch.h"
#include "input_register.h"
#include "packet_queue.h"

#include <stdio.h>

int dispatch_input_handler(packet_queue_t *packet_queue)
{
    const t_input_handler *handlers = get_input_handlers();
    input_type_e input_type = packet_queue->cli_config->input_type;

    if (input_type < 0 || input_type >= INPUT_TYPE_MAX || handlers[input_type].handler == NULL) {
        fprintf(stderr, "Invalid or unregistered input type: %d\n", input_type);
        return -1;
    }

    return handlers[input_type].handler(packet_queue);
}