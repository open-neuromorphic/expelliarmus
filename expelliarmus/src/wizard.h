#ifndef WIZARD_H
#define WIZARD_H

#include <stdio.h> 
#include <stdint.h>
#include "events.h"

// Headers delimiters.
#define HEADER_START 0x25
#define HEADER_END 0x0A

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

#define MEAS_CHECK_FILE(fp, fpath, cargo){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n", fpath); \
		cargo->events_info.dim = 0;\
		return;\
	}\
}

#define CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return -1;\
	}\
}

#define MEAS_CHECK_JUMP_HEADER(fn, cargo){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		cargo->events_info.dim = 0;\
		return;\
	}\
}

#define CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return -1;\
	}\
}

#define MEAS_CHECK_FSEEK(fn, cargo){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		cargo->events_info.dim = 0;\
		return;\
	}\
}

#define CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n"); \
		return -1;\
	}\
}

#define MEAS_CHECK_BUFF_ALLOCATION(buff, cargo){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n"); \
		cargo->events_info.dim = 0;\
		return;\
	}\
}

#define EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return -1;\
}

#define MEAS_EVENT_TYPE_NOT_RECOGNISED(event_type, cargo){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	cargo->events_info.dim = 0;\
	return;\
}

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
