#ifndef EVT3_H
#define EVT3_H

#include "events.h"
#include "wizard.h"

typedef struct {
	event_cargo_t events_info; 
	uint64_t time_high; 
	uint64_t time_low; 
	uint64_t time_high_ovfs; 
	uint64_t time_low_ovfs; 
	uint16_t base_x; 
	event_t last_event; 
} evt3_cargo_t;

DLLEXPORT void measure_evt3(const char*, evt3_cargo_t*, size_t);
DLLEXPORT void get_time_window_evt3(const char*, evt3_cargo_t*, size_t);
DLLEXPORT int read_evt3(const char*, event_t*, evt3_cargo_t*, size_t);
DLLEXPORT size_t cut_evt3(const char*, const char*, size_t, size_t);

#endif
