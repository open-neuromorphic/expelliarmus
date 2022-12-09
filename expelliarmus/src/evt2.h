#ifndef EVT2_H
#define EVT2_H

#include "events.h"
#include "wizard.h"

// EVT2 format constants.
#define EVT2_CD_OFF 0x0U
#define EVT2_CD_ON 0x1U
#define EVT2_TIME_HIGH 0x8U
#define EVT2_EXT_TRIGGER 0xAU
#define EVT2_OTHERS 0xEU
#define EVT2_CONTINUED 0xFU


typedef struct {
	event_cargo_t events_info; 
	timestamp_t last_t; 
	uint64_t time_high; 
} evt2_cargo_t;

DLLEXPORT void measure_evt2(const char*, evt2_cargo_t*, size_t);
DLLEXPORT void get_time_window_evt2(const char*, evt2_cargo_t*, size_t);
DLLEXPORT int read_evt2(const char*, event_t*, evt2_cargo_t*, size_t);
DLLEXPORT int save_evt2(const char*, event_t*, evt2_cargo_t*, size_t);
DLLEXPORT size_t cut_evt2(const char*, const char*, size_t, size_t);

#endif
