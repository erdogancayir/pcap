#ifndef INPUT_REGISTER_H
#define INPUT_REGISTER_H

#include "input_handler.h"

void register_all_input_handlers(void);
const t_input_handler *get_input_handlers(void);


#endif