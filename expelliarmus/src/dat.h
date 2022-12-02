#ifndef DAT_H
#define DAT_H 

#include "events.h"
#include "wizard.h"

typedef struct {
	event_cargo_t events_info; 
	timestamp_t last_t; 
	uint64_t time_ovfs; 
} dat_cargo_t; 

DLLEXPORT size_t measure_dat(const char*, size_t); 
DLLEXPORT int read_dat(const char*, event_t*, dat_cargo_t*, size_t); 
DLLEXPORT size_t cut_dat(const char*, const char*, size_t, size_t);

#endif
