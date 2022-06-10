#ifndef __AT_COMMAND_PROCESS__
#define __AT_COMMAND_PROCESS__

#include <Arduino.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief Bool indicating if the AT command should be processed.
 *      When set to false, the AT command will not be processed.
 */
extern bool stop_at_processing;

/**
 * @brief Processes the AT command.
 *     Reads the Serial buffer and processes the AT command. * 
 */
void process_at_commands();

#ifdef __cplusplus
} // extern "C"
#endif

#endif