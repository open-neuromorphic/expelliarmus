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

typedef struct evt2_chunk_wrap_s {
	event_array_t arr; 
	size_t bytes_read; 
	uint32_t time_high;
} evt2_chunk_wrap_t; 	

typedef struct evt3_chunk_wrap_s {
	event_array_t arr; 
	size_t bytes_read; 
	uint64_t time_high, time_low, time_high_ovfs, time_low_ovfs;
} evt3_chunk_wrap_t; 	

DLLEXPORT void read_dat_chunk(const char*, size_t, dat_chunk_wrap_t*, size_t);
DLLEXPORT void read_evt2_chunk(const char*, size_t, evt2_chunk_wrap_t*, size_t);
DLLEXPORT void read_evt3_chunk(const char*, size_t, evt3_chunk_wrap_t*, size_t);

#endif
