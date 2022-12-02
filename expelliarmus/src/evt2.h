#ifndef EVT2_H
#define EVT2_H

#include "events.h"
#include "wizard.h"

typedef struct {
	event_cargo_t events_info; 
	timestamp_t last_t; 
	uint64_t time_high; 
} evt2_cargo_t;

DLLEXPORT size_t measure_evt2(const char*, size_t);
DLLEXPORT int read_evt2(const char*, event_t*, evt2_cargo_t*, size_t);
DLLEXPORT int save_evt2(const char*, event_t*, evt2_cargo_t*, size_t);
DLLEXPORT size_t cut_evt2(const char*, const char*, size_t, size_t);

#endif
