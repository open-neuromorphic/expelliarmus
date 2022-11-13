#ifndef MUGGLE_H
#define MUGGLE_H 
//
// Thank you http://wolfprojects.altervista.org/articles/dll-in-c-for-python/ :)
// Thanks to this lines, also Windows DLL works.
#include <stdint.h>
#include <stdlib.h>
#include "expelliarmus.h"

typedef struct dat_chunk_wrap_s {
	event_array_t arr; 
	size_t bytes_read; 
} dat_chunk_wrap_t; 

DLLEXPORT dat_chunk_wrap_t read_dat_chunk(const char*, size_t, size_t, size_t);
/*
DLLEXPORT event_array_t read_evt2_chunk(const char*, size_t*, size_t, size_t*, size_t);
DLLEXPORT event_array_t read_evt3_chunk(const char*, size_t*, size_t, size_t*, size_t);
*/

#endif
