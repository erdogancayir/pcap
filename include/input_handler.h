#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

typedef enum e_input_type {
    INPUT_TYPE_INTERFACE,
    INPUT_TYPE_PCAP_FILE,
    INPUT_TYPE_MAX
} input_type_e;

typedef struct s_input_handler {
    int (*handler)(const char *input_source);
} t_input_handler;

// Ana handler fonksiyonlar
int input_handle_interface(const char *interface_name);
int input_handle_pcap_file(const char *filename);

#endif // INPUT_HANDLER_H