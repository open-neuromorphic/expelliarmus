#ifndef DAT_H
#define DAT_H 

#include "events.h"
#include "wizard.h"

// DAT format constants.
#define DAT_EVENT_2D 0x0U
#define DAT_EVENT_CD 0x0C
#define DAT_EVENT_EXT_TRIGGER 0x0E


typedef struct {
	event_cargo_t events_info; 
	uint64_t last_t; 
	uint64_t time_ovfs; 
} dat_cargo_t; 

DLLEXPORT void measure_dat(const char*, dat_cargo_t*, size_t); 
DLLEXPORT void get_time_window_dat(const char*, dat_cargo_t*, size_t); 
DLLEXPORT int read_dat(const char*, event_t*, dat_cargo_t*, size_t); 
DLLEXPORT int save_dat(const char*, event_t*, dat_cargo_t*, size_t); 
DLLEXPORT size_t cut_dat(const char*, const char*, size_t, size_t);

#endif
