#ifndef __TCP_IP_COMMANDS__
#define __TCP_IP_COMMANDS__

#include <Arduino.h>

#ifdef __cplusplus
extern "C"{
#endif

void process_tcp_server();
void register_tcp_ip_commands();

#ifdef __cplusplus
} // extern "C"
#endif

#endif