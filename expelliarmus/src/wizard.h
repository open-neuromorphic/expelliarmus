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

/********************
 * Macros.
 ********************/
#define CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n", fpath); \
		return -1;\
	}\
}

#define CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return -1;\
	}\
}

#define CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return -1;\
	}\
}

#define CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n"); \
		return -1;\
	}\
}

#define EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return -1;\
}\

#define CHECK_FWRITE(fn, expected){\
	if (expected != fn){\
		fprintf(stderr, "ERROR: fwrite failed.\n");\
		return -1;\
	}\
}

#define CUT_CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n", fpath);\
		return 0;\
	}\
}

#define CUT_CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return 0;\
	}\
}

#define CUT_CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return 0;\
	}\
}

#define CUT_CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n");\
		return 0;\
	}\
}

#define CUT_EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return 0;\
}

/********************
 * Utilities.
 ********************/
size_t jump_header(FILE*, FILE*, uint8_t);

#endif
