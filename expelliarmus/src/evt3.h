#ifndef EVT3_H
#define EVT3_H

// Library to handle EVT3 encoded binary files.

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

/** Wrap structure for the cargo information about the events tuned for the 
 *  EVT3 encoding format.
 *      
 *  @field  events_info     Information about the event stream. See "events.h".
 *  @field  time_high       The last upper 12 bits of the timestamp read.
 *  @field  time_low        The last lower 12 bits of the timestamp read.
 *  @field  time_high_ovfs  Number of overflows on the upper 12 bits of the 
 *                          timestamp.
 *  @field  time_low_ovfs   Number of overflows on the lower 12 bits of the 
 *                          timestamp.
 *  @field  base_x          The last base X address read (needed for vectorized
 *                          events).
 *  @field  last_event      The last event read.
 */
typedef struct {
	event_cargo_t events_info; 
	uint64_t time_high; 
	uint64_t time_low; 
	uint64_t time_high_ovfs; 
	uint64_t time_low_ovfs; 
	uint16_t base_x; 
	event_t last_event;
} evt3_cargo_t;

/** Function that counts the number of events encoded in the binary file 
 *  provided. 
 *  The number of events is used to allocate an external array of structures of
 *  type event_t, so that the file is re-opened successively to fill the 
 *  externally allocated array. 
 *  The size of the external array to be allocated is written to 
 *  cargo->events_info.dim.
 *
 *  @param[in]  fpath       The path to the input file.
 *  @param[out] cargo       The pointer to the information cargo structure.
 *  @param[in]  buff_size   The size of the buffer used to read the binary file.
 */
DLLEXPORT void measure_evt3(const char*, evt3_cargo_t*, size_t);

/** Function that counts the number of events to be read in the time window 
 *  duration specified.
 *  The number of events is used to allocate an external array of structures of
 *  type event_t, filled in a successive file reading. The file is read starting
 *  from the byte number stored in cargo->events_info.start_byte.
 *  The time window duration is read from cargo->events_info.time_window. 
 *  The size of the array to be allocated is saved to cargo->events_info.dim.
 *
 *  @param[in]  fpath       Path to the input file.
 *  @param[out] cargo       The pointer to the information cargo structure. 
 *  @param[in]  buff_size   The size of the buffer used to read the binary file.
 */
DLLEXPORT void get_time_window_evt3(const char*, evt3_cargo_t*, size_t);

/** Function that fills the array provided with the events from the binary file.
 *  arr is supposed to be an array of size cargo->events_info.dim and type
 *  event_t.
 *  When the entire file has been read, cargo->events_info.finished is set to 1.
 *
 *  @param[in]  fpath       Path to the input file.
 *  @param[out] arr         The event array, passed as a pointer to event_t 
 *                          structures. Allocated externally.
 *  @param[in]  cargo       The pointer to the information cargo structure. 
 *  @param[in]  buff_size   The size of the buffer used to read the binary file.
 *
 *  @return     status      A flag that when different from 0, indicates that 
 *                          there has been some error while filling the input 
 *                          array or reading the file.
 */
DLLEXPORT int read_evt3(const char*, event_t*, evt3_cargo_t*, size_t);

/** Function that writes to a binary file the array provided in input using 
 *  EVT3 encoding.
 *
 *  @param[in]      fpath       Path to the output file.
 *  @param[in]      arr         The event array, passed as an array of event_t 
 *                              structures. 
 *  @param[in]      cargo       The pointer to the information cargo structure. 
 *  @param[in]      buff_size   The size of the buffer used to write the binary 
 *                              file.
 *
 *  @return         status      A flag that when different from 0, indicates 
 *                              that there has been some error while reading 
 *                              the input array or writing the file.
 */
DLLEXPORT int save_evt3(const char*, event_t*, evt3_cargo_t*, size_t);

/** Function that copies part of the input file to an output one such that the 
 *  recording encoded has a duration specified in the new_duration parameter. 
 *
 *  @param[in]  fpath_in        Input file path.
 *  @param[in]  fpath_out       Output file path.
 *  @param[in]  new_duration    New duration of the encoding saved to the output
 *                              file expressed in milliseconds.
 *  @param[in]  buff_size       The size of the buffer used to read and write 
 *                              the binary files.
 *
 *  @return     dim             The number of events written to the output file.
 */                    
DLLEXPORT size_t cut_evt3(const char*, const char*, size_t, size_t);

#endif
