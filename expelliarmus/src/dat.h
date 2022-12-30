#ifndef DAT_H
#define DAT_H 

#include "events.h"
#include "wizard.h"

// DAT format constants.
#define DAT_EVENT_2D 0x0U
#define DAT_EVENT_CD 0x0C
#define DAT_EVENT_EXT_TRIGGER 0x0E

/** Wrap structure for the cargo information about the events tuned for the 
 *  DAT encoding format.
 *      
 *  @field  events_info Information about the event stream. See "events.h".
 *  @field  last_t      Last timestamp read.
 *  @field  time_ovfs   Number of overflows in the timestamps occured in the
 *                      event stream, for recordings with t >= 2^32.
 */
typedef struct {
	event_cargo_t events_info; 
	uint64_t last_t; 
	uint64_t time_ovfs; 
} dat_cargo_t; 

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
DLLEXPORT void measure_dat(const char*, dat_cargo_t*, size_t); 

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
DLLEXPORT void get_time_window_dat(const char*, dat_cargo_t*, size_t); 

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
DLLEXPORT int read_dat(const char*, event_t*, dat_cargo_t*, size_t); 

/** Function that writes to a binary file the array provided in input using 
 *  DAT encoding.
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
DLLEXPORT int save_dat(const char*, event_t*, dat_cargo_t*, size_t); 

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
DLLEXPORT size_t cut_dat(const char*, const char*, size_t, size_t);

#endif
