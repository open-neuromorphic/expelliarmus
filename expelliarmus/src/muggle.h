#ifndef MUGGLE_H
#define MUGGLE_H 

#include <stdint.h>
#include <stdlib.h>
#include "events.h"

// Thank you http://wolfprojects.altervista.org/articles/dll-in-c-for-python/ :)
// Thanks to this lines, also Windows DLL works.
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else 
#define DLLEXPORT
#endif

typedef struct dat_chunk_s {
	event_array_t arr; 
	size_t bytes_read; 
	size_t file_size; 
} dat_chunk_t; 

typedef struct evt2_chunk_s {
	event_array_t arr; 
	size_t bytes_read; 
	size_t file_size; 
	uint64_t time_high;
} evt2_chunk_t; 	

typedef struct evt3_chunk_s {
	event_array_t arr; 
	size_t bytes_read; 
	size_t file_size; 
	uint16_t base_x;
	uint64_t time_high, time_low, time_high_ovfs, time_low_ovfs;
	event_t event_tmp; 
} evt3_chunk_t; 	

DLLEXPORT void read_dat_chunk(const char*, size_t, dat_chunk_t*, size_t);
DLLEXPORT void read_evt2_chunk(const char*, size_t, evt2_chunk_t*, size_t);
DLLEXPORT void read_evt3_chunk(const char*, size_t, evt3_chunk_t*, size_t);

#endif
