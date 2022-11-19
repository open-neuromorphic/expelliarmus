#ifndef EVENTS_H 
#define EVENTS_H

#include <stdlib.h>
#include <stdint.h>

// Thank you http://wolfprojects.altervista.org/articles/dll-in-c-for-python/ :)
// Thanks to this lines, also Windows DLL works.
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else 
#define DLLEXPORT
#endif

#define DEFAULT_ARRAY_DIM 8192

typedef int64_t timestamp_t;  
typedef int16_t pixel_t; 
typedef uint8_t polarity_t; 

typedef struct event_s {
	timestamp_t t; 
	pixel_t x, y; 
	polarity_t p; 
} event_t; 

typedef struct event_array_s {
	timestamp_t* t_arr; 
	pixel_t* x_arr; 
	pixel_t* y_arr; 
	polarity_t* p_arr; 
	size_t dim; 
	size_t allocated_space;
} event_array_t; 

#define CHECK_TIMESTAMP_MONOTONICITY(timestamp, prev_timestamp){\
	if (((uint64_t)timestamp) < ((uint64_t)prev_timestamp))\
		fprintf(stderr, "WARNING: the timestamps are not monotonic. Current: %lu; previous:%lu.\n", ((uint64_t)timestamp), ((uint64_t)prev_timestamp));\
}	

DLLEXPORT void free_event_array(event_array_t*); 
DLLEXPORT unsigned int is_void_event_array(event_array_t*);
event_array_t realloc_event_array(event_array_t*, size_t);
event_array_t malloc_event_array(size_t);
void add_event(const event_t*, event_array_t*, size_t); 
event_array_t void_event_array();
event_t void_event();

#endif
