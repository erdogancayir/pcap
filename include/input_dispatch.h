#ifndef INPUT_DISPATCH_H
#define INPUT_DISPATCH_H

#include "input_handler.h"
#include "cli_config.h"

int dispatch_input_handler(packet_queue_t *packet_queue);
const char *get_input_type_string(input_type_e input_type);

#endif