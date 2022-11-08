#ifndef EXPELLIARMUS_H
#define EXPELLIARMUS_H

#ifdef _WIN32
#define DllExport __declspec( dllexport )
#endif

#include <stdio.h> 
#include <stdint.h>

#define DEFAULT_ARRAY_DIM 16000

#define T_POS 0U
#define X_POS 1U
#define Y_POS 2U
#define P_POS 3U

#define DAT_EVENT_2D 0x0U
#define DAT_EVENT_CD 0x0C
#define DAT_EVENT_EXT_TRIGGER 0x0E

#define EVT2_CD_OFF 0x0U
#define EVT2_CD_ON 0x1U
#define EVT2_TIME_HIGH 0x8U
#define EVT2_EXT_TRIGGER 0xAU
#define EVT2_OTHERS 0xEU
#define EVT2_CONTINUED 0xFU

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

typedef uint64_t event_t; 
typedef event_t* event_array_t; 

struct event_s {
	event_t t; 
	event_t x; 
	event_t y; 
	event_t p; 
}; 

size_t cut_dat(const char*, const char*, size_t, size_t);
size_t cut_evt2(const char*, const char*, size_t, size_t);
size_t cut_evt3(const char*, const char*, size_t, size_t);
event_array_t read_dat(const char*, size_t*, size_t); 
event_array_t read_evt2(const char*, size_t*, size_t);
event_array_t read_evt3(const char*, size_t*, size_t);
void append_event(const struct event_s*, event_array_t*, size_t*, size_t*); 
void free_event_array(event_array_t);

#endif
