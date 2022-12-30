#ifndef WIZARD_H
#define WIZARD_H

// Macros common to the encoding specific libraries.

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

/** Macro for checking correct file opening during read_<encoding>() execution.
 *  If the file pointer is NULL, an error is returned.
 */
#define CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n",\
                fpath);\
		return -1;\
	}\
}

/** Macro for checking correct file opening during meas_<encoding>() execution.
 *  If the file pointer is NULL, an error is returned and the array size is
 *  set to 0.
 */
#define MEAS_CHECK_FILE(fp, fpath, cargo){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n",\
                fpath);\
		cargo->events_info.dim = 0;\
		return;\
	}\
}

/** Macro for checking that the binary file header has been properly handled.
 *  Prints out an error if something goes wrong while reading the header.
 *  Used in read_<encoding>() functions.
 */
#define CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return -1;\
	}\
}

/** Macro for checking that the binary file header has been properly handled.
 *  Prints out an error if something goes wrong while reading the header.
 *  Used in meas_<encoding>() functions.
 */
#define MEAS_CHECK_JUMP_HEADER(fn, cargo){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		cargo->events_info.dim = 0;\
		return;\
	}\
}

/** Macro to check that fseek() executes correctly.
 *  To read a file in chunks, a start_byte parameter is used start from the last
 *  point in the file; fseek() is used for this purpose. 
 *  An error is printed and returned if something goes wrong.
 *  Used in read_<encoding>() functions.
 */
#define CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return -1;\
	}\
}

/** Macro to check that fseek() executes correctly.
 *  To read a file in chunks, a start_byte parameter is used start from the last
 *  point in the file; fseek() is used for this purpose. 
 *  An error is printed and returned if something goes wrong.
 *  Used in meas_<encoding>() functions.
 */
#define MEAS_CHECK_FSEEK(fn, cargo){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		cargo->events_info.dim = 0;\
		return;\
	}\
}

/** Macro for memory allocation checking of the buffer.
 *  Used to check that the buffer used to read the binary files is correctly
 *  allocated through malloc.
 *  Used in read_<encoding>() functions.
 */
#define CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr,\
                "ERROR: the read buffer could not be allocated.\n");\
		return -1;\
	}\
}

/** Macro for memory allocation checking of the buffer.
 *  Used to check that the buffer used to read the binary files is correctly
 *  allocated through malloc. The number of events read is set to 0.
 *  Used in meas_<encoding>() functions.
 */
#define MEAS_CHECK_BUFF_ALLOCATION(buff, cargo){\
	if (buff==NULL){\
		fprintf(stderr,\
                "ERROR: the read buffer could not be allocated.\n");\
		cargo->events_info.dim = 0;\
		return;\
	}\
}

/** Macro used to signal unexpected events.
 *  If a non listed event comes up in the stream, an error is returned and 
 *  printed out.
 *  Used in read_<encoding>() functions.
 */
#define EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return -1;\
}

/** Macro used to signal unexpected events.
 *  If a non listed event comes up in the stream, an error is returned and 
 *  printed out. The number of events read is set to 0.
 *  Used in meas_<encoding>() functions.
 */
#define MEAS_EVENT_TYPE_NOT_RECOGNISED(event_type, cargo){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	cargo->events_info.dim = 0;\
	return;\
}

/** Macro used to check tha fwrite executes properly.
 *  Used in save_<encoding>() functions.
 */
#define CHECK_FWRITE(fn, expected){\
	if (expected != fn){\
		fprintf(stderr, "ERROR: fwrite failed.\n");\
		return -1;\
	}\
}

/** Macro for checking correct file opening during cut_<encoding>() execution.
 *  If the file pointer is NULL, an error is returned.
 */
#define CUT_CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr,\
                "ERROR: the input file \"%s\" could not be opened.\n", fpath);\
		return 0;\
	}\
}

/** Macro for checking that the binary file header has been properly handled.
 *  Prints out an error if something goes wrong while reading the header.
 *  Used in cut_<encoding>() functions.
 */
#define CUT_CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return 0;\
	}\
}

/** Macro to check that fseek() executes correctly.
 *  To read a file in chunks, a start_byte parameter is used start from the last
 *  point in the file; fseek() is used for this purpose. 
 *  An error is printed and returned if something goes wrong.
 *  Used in cut_<encoding>() functions.
 */
#define CUT_CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return 0;\
	}\
}

/** Macro for memory allocation checking of the buffer.
 *  Used to check that the buffer used to read the binary files is correctly
 *  allocated through malloc.
 *  Used in cut_<encoding>() functions.
 */
#define CUT_CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr,\
                "ERROR: the read buffer could not be allocated.\n");\
		return 0;\
	}\
}

/** Macro used to signal unexpected events.
 *  If a non listed event comes up in the stream, an error is returned and 
 *  printed out.
 *  Used in cut_<encoding>() functions.
 */
#define CUT_EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return 0;\
}

/** Function to handle binary files header.
 *  The header of the file is skipped through this function.
 *
 *  @param[in]  fp_in       Input file pointer.
 *  @param[in]  fp_out      Output file pointer
 *  @param[in]  copy_file   Flag used to choose if the header has to be copied
 *                          to fpath_out or not.
 *
 *  @return     bytes_read  The number of bytes read while skipping the header.         
 */
size_t jump_header(FILE*, FILE*, uint8_t);

#endif
