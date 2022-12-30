#ifndef EVENTS_H 
#define EVENTS_H

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Data types used for the event data.
 */
typedef int64_t timestamp_t;  
typedef int16_t address_t; 
typedef uint8_t polarity_t; 

/**
 * @brief Structure of an event in a recording.
 */
typedef struct event_s {
	timestamp_t t; 
	address_t x; 
	address_t y; 
	polarity_t p; 
} event_t; 

/**
 * @brief Additional information for the Python and C code about the event array.
 */
typedef struct {
	size_t dim;
	uint8_t is_chunk; 
	size_t time_window; 
	uint8_t is_time_window; 
	size_t start_byte;
	uint8_t finished; 
} event_cargo_t; 

/**
 * @brief Macro to check that the event stream is monotonic in the timestamps.
 */
#define CHECK_TIMESTAMP_MONOTONICITY(timestamp, prev_timestamp){\
	if ((timestamp) < (prev_timestamp))\
		fprintf(stderr, "WARNING: the timestamps are not monotonic. Current: %ld; previous:%ld.\n", timestamp, prev_timestamp);\
}	

#endif
