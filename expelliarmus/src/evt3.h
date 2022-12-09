#ifndef EVT3_H
#define EVT3_H

#include "events.h"
#include "wizard.h"

// EVT3 format constants.
#define EVT3_EVT_ADDR_Y 0x0U
#define EVT3_EVT_ADDR_X 0x2U
#define EVT3_VECT_BASE_X 0x3U
#define EVT3_VECT_12 0x4U
#define EVT3_VECT_8 0x5U
#define EVT3_TIME_LOW 0x6U
#define EVT3_CONTINUED_4 0x7U
#define EVT3_TIME_HIGH 0x8U
#define EVT3_EXT_TRIGGER 0xCU
#define EVT3_OTHERS 0xEU
#define EVT3_CONTINUED_12 0xFU


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
DLLEXPORT int save_evt3(const char*, event_t*, evt3_cargo_t*, size_t);
DLLEXPORT size_t cut_evt3(const char*, const char*, size_t, size_t);

#endif
