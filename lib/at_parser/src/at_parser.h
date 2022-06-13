#ifndef AT_PARSER_H
#define AT_PARSER_H

#include "my_string.h"

#define AT_MAX_TEMP_STRING	512

typedef char (*at_callback)(unsigned char *value);

typedef struct _at_command
{
    unsigned long hash;
    const char *name;
    at_callback setter;
    at_callback getter;
    at_callback test;
    at_callback execute;
} AT_COMMAND;

#define AT_OK 		                0
#define AT_ERROR 	                1

#define AT_ERROR_STRING                 "ERROR"
#define AT_OK_STRING                    "OK"

#define AT_PARSER_STATE_COMMAND 	0
#define AT_PARSER_STATE_TEST		1
#define AT_PARSER_STATE_READ		2
#define AT_PARSER_STATE_WRITE		3
#define AT_PARSER_STATE_EQUAL		4

#ifndef AT_COMMAND_MARKER
#define AT_COMMAND_MARKER "AT+"
#endif

#ifndef AT_COMMANDS_NUM
#define AT_COMMANDS_NUM 50
#endif

#ifdef __cplusplus
extern "C"{
#endif

extern AT_COMMAND at_registered_commands[AT_COMMANDS_NUM];

unsigned long at_hash(const char *str);
void at_register_command(const char *command, at_callback getter, at_callback setter, at_callback test, at_callback execute);
char at_parse_line(const char *line, unsigned char *ret);

#ifdef __cplusplus
} // extern "C"
#endif

#endif