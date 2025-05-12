#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>
#include <pcap.h>

extern volatile sig_atomic_t stop_capture;

void sigint_handler(int signo);
void set_pcap_handle(pcap_t *handle);

#endif