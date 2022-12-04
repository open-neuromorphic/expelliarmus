#include "evt3.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

DLLEXPORT void measure_evt3(const char* fpath, evt3_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	MEAS_CHECK_FILE(fp, fpath, cargo); 
	
	// Jumping over the headers.
	if (cargo->events_info.start_byte == 0){
		MEAS_CHECK_JUMP_HEADER((cargo->events_info.start_byte = jump_header(fp, NULL, 0U)), cargo);
		cargo->events_info.end_byte = cargo->events_info.start_byte; 
	} else {
		cargo->events_info.start_byte = cargo->events_info.end_byte; 
		MEAS_CHECK_FSEEK(fseek(fp, cargo->events_info.start_byte, SEEK_SET), cargo); 
	}

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	MEAS_CHECK_BUFF_ALLOCATION(buff, cargo); 
	
	// Indices to read the file.
	size_t values_read=0, j=0, dim=0; 
	// Byte that identifies the event type.
	uint8_t event_type; 

	// Counters used to keep track of number of events_info encoded in vectors and of the base x address of these.
	uint16_t k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary values to handle overflows.
	uint64_t buff_tmp=0;
	uint8_t loop_condition = 1; 

	uint64_t last_t = (uint64_t) cargo->last_event.t, cargo_last_t = last_t; 
	uint64_t time_high=cargo->time_high, time_low=cargo->time_low, time_high_ovfs=cargo->time_high_ovfs, time_low_ovfs=cargo->time_low_ovfs; 

	// Reading the file.
	while (loop_condition && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; loop_condition && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					break; 

				case EVT3_EVT_ADDR_X:
					dim++; 
					break; 

				case EVT3_VECT_BASE_X:
					break; 

				case EVT3_VECT_12:
					num_vect_events = 12; 
					buff_tmp = (uint16_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events == 0){
						num_vect_events = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events; k++){
						if (buff_tmp & (1U<<k)){
							dim++; 
						}
					}
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t) buff[j] & mask_12b; 
					if (buff_tmp < time_low)
						time_low_ovfs++; 
					time_low = buff_tmp; 
					last_t = (time_high_ovfs << 24) + ((time_high + time_low_ovfs) << 12) + time_low; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t) buff[j] & mask_12b; 
					if (buff_tmp < time_high)
						time_high_ovfs++; 
					time_high = buff_tmp; 
					last_t = (time_high_ovfs << 24) + ((time_high + time_low_ovfs) << 12) + time_low; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					MEAS_EVENT_TYPE_NOT_RECOGNISED(event_type, cargo); 
			}
			if (cargo->events_info.is_time_window)
				loop_condition = cargo->events_info.time_window > (last_t - cargo_last_t); 
		}
		cargo->events_info.end_byte += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = dim; 
	return; 
}

DLLEXPORT int read_evt3(const char* fpath, event_t* arr, evt3_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	if (cargo->events_info.start_byte == 0){
		CHECK_JUMP_HEADER((cargo->events_info.start_byte = jump_header(fp, NULL, 0U))); 
	} else {
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.start_byte, SEEK_SET)); 
	}
	size_t byte_pt = cargo->events_info.start_byte; 

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff); 
	
	// Indices to read the file.
	size_t values_read=0, j=0, i=0; 
	// Byte that identifies the event type.
	uint8_t event_type; 

	// Counters used to keep track of number of events_info encoded in vectors and of the base x address of these.
	uint16_t k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary values to handle overflows.
	uint64_t buff_tmp=0;
   	timestamp_t timestamp=0; 

	uint8_t loop_condition = 1; 
	// Reading the file.
	while (loop_condition && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; loop_condition && j < values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					arr[i].y = (address_t)(buff[j] & mask_11b);
					cargo->last_event.y = arr[i].y; 
					break; 

				case EVT3_EVT_ADDR_X:
					// p
					arr[i].p = (polarity_t) ((buff[j] >> 11) & 0x1U); 
					cargo->last_event.p = arr[i].p; 
					// y
					arr[i].y = cargo->last_event.y;
					// t
					arr[i].t = cargo->last_event.t;
					// x
					arr[i++].x = (address_t)(buff[j] & mask_11b);
					break; 

				case EVT3_VECT_BASE_X:
					arr[i].p = (polarity_t) ((buff[j] >> 11) & 0x1U); 
					cargo->last_event.p = arr[i].p; 
					cargo->base_x = (uint16_t)(buff[j] & mask_11b);
					break; 

				case EVT3_VECT_12:
					num_vect_events = 12; 
					buff_tmp = (uint16_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events == 0){
						num_vect_events = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events; k++){
						if (buff_tmp & (1U<<k)){
							// y
							arr[i].y = cargo->last_event.y;
							// p
							arr[i].p = cargo->last_event.p;
							// t
							arr[i].t = cargo->last_event.t;
							// x
							arr[i++].x = (address_t)(cargo->base_x + k); 
						}
					}
					cargo->base_x += num_vect_events; 
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < cargo->time_low) // Overflow.
						cargo->time_low_ovfs++; 
					cargo->time_low = buff_tmp; 
					timestamp = (timestamp_t)((cargo->time_high_ovfs<<24) + ((cargo->time_high+cargo->time_low_ovfs)<<12) + cargo->time_low);
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, cargo->last_event.t);
					arr[i].t = timestamp; 
					cargo->last_event.t = timestamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < cargo->time_high) // Overflow.
						cargo->time_high_ovfs++; 
					cargo->time_high = buff_tmp; 
					timestamp = (timestamp_t)((cargo->time_high_ovfs<<24) + ((cargo->time_high+cargo->time_low_ovfs)<<12) + cargo->time_low);
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, cargo->last_event.t);
					arr[i].t = timestamp; 
					cargo->last_event.t = timestamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}

			if (cargo->events_info.is_time_window)
				loop_condition = byte_pt < cargo->events_info.end_byte; 
			else
				loop_condition = i < cargo->events_info.dim; 
		}
		byte_pt += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = i; 
	if (cargo->events_info.is_chunk)
		cargo->events_info.start_byte = byte_pt; 
	if (i==0)
		return -1; 
	return 0; 
}

/*
DLLEXPORT int save_evt3(const char* fpath, event_t* arr, evt3_cargo_t* cargo, size_t buff_size){
	char header[150]; 
	sprintf(header, "%c This EVT3 file has been generated through expelliarmus (https://github.com/fabhertz95/expelliarmus.git) %c%c evt 3.0 %c", (char)HEADER_START, (char)HEADER_END, (char)HEADER_START, (char)HEADER_END); 
	const size_t header_len = strlen(header); 
	FILE* fp; 
	if (cargo->events_info.bytes_done == 0){
		fp = fopen(fpath, "wb");	
		CHECK_FILE(fp, fpath); 
		CHECK_FWRITE((cargo->events_info.bytes_done=fwrite(header, sizeof(char), header_len, fp)), header_len); 
	} else {
		fp = fopen(fpath, "ab"); 
		CHECK_FILE(fp, fpath); 
	}

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff); 
	
	// Indices to read the file.
	size_t j=0, i=0, k=0, i_start=0; 
	// Byte that identifies the event type.
	uint8_t event_type; 

	// Counters used to keep track of number of events_info encoded in vectors and of the base x address of these.
	uint16_t k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary values to handle overflows.
	uint64_t buff_tmp=0;
   	timestamp_t timestamp=0; 
	// Reading the file.
	while (i < cargo->events_info.dim){
		// First event.
		if (cargo->events_info.bytes_done == header_len && i==0){
			// Y address.
			buff[0] = ((uint16_t) EVT3_EVT_ADDR_Y) << 12; 
			buff[0] |= ((uint16_t) arr[i].y) & mask_11b; 
			cargo->last_event.y = arr[i].y;
			// Time low.
			buff[1] = ((uint16_t) EVT3_TIME_HIGH) << 12; 
			buff[1] |= (uint16_t) ((arr[i].t >> 12) & mask_12b); 
			// Time high.
			buff[2] = ((uint16_t) EVT3_TIME_LOW) << 12; 
			buff[2] |= (uint16_t) (arr[i].t & mask_12b); 
			cargo->last_event.t = arr[i++].t;
			// Assigning negative value to X so that we denote this as first event.
			cargo->last_event.x = -1;
			CHECK_FWRITE(fwrite(buff, sizeof(*buff), 3, fp), 3); 
			cargo->events_info.bytes_done += 3 * sizeof(*buff); 
		}
		for (j=0; i < cargo->events_info.dim && j < buff_size; j++){
			// Check for vectorized event.
			if (cargo->last_event.x == arr[i].x){
				i_start = i; 
				while (cargo->last_event.x == arr[i++].x && i-i_start < 12); 
				if (i-i_start == 12){
				} else if (i-i_start >= 8){
					buff[j] = 
					for (k=i_start; k < i_start + 8; k++){
					}	
				} else {
				}
			}
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					arr[i].y = (address_t)(buff[j] & mask_11b);
					cargo->last_event.y = arr[i].y; 
					break; 

				case EVT3_EVT_ADDR_X:
					// p
					arr[i].p = (polarity_t) ((buff[j] >> 11)%2); 
					cargo->last_event.p = arr[i].p; 
					// y
					arr[i].y = cargo->last_event.y;
					// t
					arr[i].t = cargo->last_event.t;
					// x
					arr[i++].x = (address_t)(buff[j] & mask_11b);
					break; 

				case EVT3_VECT_BASE_X:
					arr[i].p = (polarity_t) ((buff[j] >> 11)%2); 
					cargo->last_event.p = arr[i].p; 
					cargo->base_x = (uint16_t)(buff[j] & mask_11b);
					break; 

				case EVT3_VECT_12:
					num_vect_events = 12; 
					buff_tmp = (uint16_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events == 0){
						num_vect_events = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events; k++){
						if (buff_tmp & (1U<<k)){
							// y
							arr[i].y = cargo->last_event.y;
							// p
							arr[i].p = cargo->last_event.p;
							// t
							arr[i].t = cargo->last_event.t;
							// x
							arr[i++].x = (address_t)(cargo->base_x + k); 
						}
					}
					cargo->base_x += num_vect_events; 
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < cargo->time_low) // Overflow.
						cargo->time_low_ovfs++; 
					cargo->time_low = buff_tmp; 
					timestamp = (timestamp_t)((cargo->time_high_ovfs<<24) + ((cargo->time_high+cargo->time_low_ovfs)<<12) + cargo->time_low);
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, cargo->last_event.t);
					arr[i].t = timestamp; 
					cargo->last_event.t = timestamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < cargo->time_high) // Overflow.
						cargo->time_high_ovfs++; 
					cargo->time_high = buff_tmp; 
					timestamp = (timestamp_t)((cargo->time_high_ovfs<<24) + ((cargo->time_high+cargo->time_low_ovfs)<<12) + cargo->time_low);
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, cargo->last_event.t);
					arr[i].t = timestamp; 
					cargo->last_event.t = timestamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
		cargo->events_info.bytes_done += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = i; 
	if (i==0)
		return -1; 
	return 0; 
}
*/

DLLEXPORT size_t cut_evt3(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CUT_CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "w+b"); 
	CUT_CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	CUT_CHECK_JUMP_HEADER(jump_header(fp_in, fp_out, 1U)); 	

	// Buffer to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CUT_CHECK_BUFF_ALLOCATION(buff); 

	// Indices to access the binary file.
	size_t values_read=0, j=0, i=0; 

	// Temporary values to keep track of vectorized events_info.
	uint64_t buff_tmp=0, k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_8b = 0xFFU, mask_12b = 0xFFFU; 
	// Flags to recognise event type and end of file.
	uint8_t event_type=0, recording_finished=0, last_events_info_acquired=0, get_out=0; 
	// Temporary values.
	uint64_t first_timestamp=0, timestamp=0, time_high=0, time_high_ovfs=0, time_low=0, time_low_ovfs=0; 
	// Converting duration to microseconds.
	new_duration *= 1000; 

	while (!get_out && (values_read = fread(buff, sizeof(*buff), buff_size, fp_in)) > 0){
		for (j=0; !get_out && j<values_read; j++){
			// Getting the event type. 
			event_type = (buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					break; 

				case EVT3_EVT_ADDR_X:
					if (recording_finished)
						last_events_info_acquired = 1; 
					i++; 
					break; 

				case EVT3_VECT_BASE_X:
					break; 

				case EVT3_VECT_12:
					num_vect_events = 12; 
					buff_tmp = (uint64_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events == 0){
						num_vect_events = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events; k++){
						if (buff_tmp%2)
							i++; 
						buff_tmp = buff_tmp >> 1; 
					}
					num_vect_events = 0; 
					if (recording_finished)
						last_events_info_acquired = 1; 
					break; 

				case EVT3_TIME_LOW:
					if ((timestamp - first_timestamp) >= (uint64_t)new_duration){
						recording_finished = 1;
						if (last_events_info_acquired)
							get_out = 1; 
					}
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < time_low) // Overflow.
						time_low_ovfs++; 
					time_low = buff_tmp; 
					timestamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					if (i==0)
						first_timestamp = timestamp; 
					break; 

				case EVT3_TIME_HIGH:
					if ((timestamp - first_timestamp) >= (uint64_t)new_duration){
						recording_finished = 1;
						if (last_events_info_acquired)
							get_out = 1; 
					}
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < time_high) // Overflow.
						time_high_ovfs++; 
					time_high = buff_tmp; 
					timestamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					if (i==0)
						first_timestamp = timestamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					CUT_EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
		CHECK_FWRITE(fwrite(buff, sizeof(*buff), j, fp_out), j); 
	}
	fclose(fp_in); 
	fclose(fp_out); 
	free(buff); 
	return i; 
}