#ifndef WIZARD_H
#define WIZARD_H

#include <stdio.h> 
#include <stdint.h>
#include "events.h"

// Headers delimiters.
#define HEADER_START 0x25
#define HEADER_END 0x0A

// DAT format constants.
#define DAT_EVENT_2D 0x0U
#define DAT_EVENT_CD 0x0C
#define DAT_EVENT_EXT_TRIGGER 0x0E

// EVT2 format constants.
#define EVT2_CD_OFF 0x0U
#define EVT2_CD_ON 0x1U
#define EVT2_TIME_HIGH 0x8U
#define EVT2_EXT_TRIGGER 0xAU
#define EVT2_OTHERS 0xEU
#define EVT2_CONTINUED 0xFU

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

// Thank you http://wolfprojects.altervista.org/articles/dll-in-c-for-python/ :)
// Thanks to this lines, also Windows DLL works.
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else 
#define DLLEXPORT
#endif

typedef struct {
	event_cargo_t events_info; 
	timestamp_t last_t; 
	uint64_t time_ovfs; 
} dat_cargo_t; 

typedef struct {
	event_cargo_t events_info; 
	timestamp_t last_t; 
	uint64_t time_high; 
} evt2_cargo_t;

typedef struct {
	event_cargo_t events_info; 
	uint64_t time_high; 
	uint64_t time_low; 
	uint64_t time_high_ovfs; 
	uint64_t time_low_ovfs; 
	uint16_t base_x; 
	event_t last_event; 
} evt3_cargo_t;

/********************
 * DAT
 ********************/
DLLEXPORT size_t measure_dat(const char*, size_t); 
DLLEXPORT int read_dat(const char*, event_t*, dat_cargo_t*, size_t); 
DLLEXPORT size_t cut_dat(const char*, const char*, size_t, size_t);
/********************
 * EVT2
 ********************/
DLLEXPORT size_t measure_evt2(const char*, size_t);
DLLEXPORT int read_evt2(const char*, event_t*, evt2_cargo_t*, size_t);
DLLEXPORT int compress_evt2(const char*, event_t*, evt2_cargo_t*, size_t);
DLLEXPORT size_t cut_evt2(const char*, const char*, size_t, size_t);
/********************
 * EVT3
 ********************/
DLLEXPORT size_t measure_evt3(const char*, size_t);
DLLEXPORT int read_evt3(const char*, event_t*, evt3_cargo_t*, size_t);
DLLEXPORT size_t cut_evt3(const char*, const char*, size_t, size_t);
/********************
 * Utilities.
 ********************/
size_t jump_header(FILE*, FILE*, uint8_t);

#endif
