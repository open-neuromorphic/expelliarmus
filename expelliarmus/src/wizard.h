#ifndef WIZARD_H
#define WIZARD_H

#include <stdio.h> 
#include <stdint.h>

#define DEFAULT_ARRAY_DIM 8192

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

// Thank you http://wolfprojects.altervista.org/articles/dll-in-c-for-python/ :)
// Thanks to this lines, also Windows DLL works.
#define DEFAULT_ARRAY_DIM 8192

#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else 
#define DLLEXPORT
#endif

#define CHECK_ALLOCATION(pt) {\
	if (pt == NULL){\
		fprintf(stderr, "Error during dinamic array memory allocation.\n");\
		exit(EXIT_FAILURE);\
	}\
}

#define CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "Error while opening the file \"%s\".\n", fpath);\
		exit(EXIT_FAILURE);\
	}\
}

#define CHECK_TIMESTAMP_MONOTONICITY(timestamp, prev_timestamp){\
	if (((uint64_t)timestamp) < ((uint64_t)prev_timestamp))\
		fprintf(stderr, "WARNING: the timestamps are not monotonic. Current: %lu; previous:%lu.\n", ((uint64_t)timestamp), ((uint64_t)prev_timestamp));\
}	

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

DLLEXPORT void free_event_array(event_array_t*); 
DLLEXPORT size_t cut_dat(const char*, const char*, size_t, size_t);
DLLEXPORT size_t cut_evt2(const char*, const char*, size_t, size_t);
DLLEXPORT size_t cut_evt3(const char*, const char*, size_t, size_t);
DLLEXPORT event_array_t read_dat(const char*, size_t); 
DLLEXPORT event_array_t read_evt2(const char*, size_t);
DLLEXPORT event_array_t read_evt3(const char*, size_t);
void append_event(const event_t*, event_array_t*, size_t); 

#endif
