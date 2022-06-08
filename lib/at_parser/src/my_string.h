#ifndef __MY_STRING__
#define __MY_STRING__

#include "types.h"

#ifdef __cplusplus
extern "C"{
#endif

int16_t ms_str_find(const char *haystack, const char * needle);
void ms_array_slice_to_string(const char * array, uint16_t start, uint16_t end, char *ret);
uint16_t ms_strlen(const char * string);

#ifdef __cplusplus
} // extern "C"
#endif

#endif