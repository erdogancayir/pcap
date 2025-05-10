#include "input_handler.h"
#include "cli_config.h"
#include "input_register.h"
#include "input_dispatch.h"
#include "cli_parser.h"
#include <stdio.h>

int dispatch_input_handler(input_type_e type, const char *input_source) {
    const t_input_handler *handlers = get_input_handlers();

    if (type < 0 || type >= INPUT_TYPE_MAX || handlers[type].handler == NULL) {
        fprintf(stderr, "Invalid or unregistered input type: %d\n", type);
        return -1;
    }

    return handlers[type].handler(input_source);
}