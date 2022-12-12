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
	} else {
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
	uint16_t buff_tmp=0;


	// Reading the file.
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
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
						buff_tmp = (uint16_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events; k++){
						if (buff_tmp & (1U<<k)){
							dim++; 
						}
					}
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
				case EVT3_TIME_HIGH:
				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
				case EVT3_CONTINUED_4:
					break; 

				default:
					MEAS_EVENT_TYPE_NOT_RECOGNISED(event_type, cargo); 
			}
		}
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = dim; 
	if (values_read==0)
		cargo->events_info.finished = 1;
	return; 
}

DLLEXPORT void get_time_window_evt3(const char* fpath, evt3_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	MEAS_CHECK_FILE(fp, fpath, cargo); 
	
	// Jumping over the headers.
	if (cargo->events_info.start_byte == 0){
		MEAS_CHECK_JUMP_HEADER((cargo->events_info.start_byte = jump_header(fp, NULL, 0U)), cargo);
	} else {
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

	uint64_t last_t = 0, first_t = 0; 
	uint64_t time_high=cargo->time_high, time_low=cargo->time_low, time_high_ovfs=cargo->time_high_ovfs, time_low_ovfs=cargo->time_low_ovfs; 
	uint64_t time_window = cargo->events_info.time_window; 
	uint8_t first_run=1, loop_condition_flag=1; 

	// Reading the file.
	while (loop_condition_flag && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; loop_condition_flag && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					break; 

				case EVT3_EVT_ADDR_X:
					dim++; 
					loop_condition_flag = (time_window > last_t - first_t);
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
						if (buff_tmp & (1U<<k)){
							dim++; 
						}
					}
					num_vect_events = 0; 
					loop_condition_flag = (time_window > last_t - first_t);
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t) buff[j] & mask_12b; 
					if (buff_tmp < time_low)
						time_low_ovfs++; 
					time_low = buff_tmp; 
					last_t = (time_high_ovfs << 24) + ((time_high + time_low_ovfs) << 12) + time_low; 
					if (first_run){
						first_t = last_t; 
						first_run = 0; 
					}
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t) buff[j] & mask_12b; 
					if (buff_tmp < time_high)
						time_high_ovfs++; 
					time_high = buff_tmp; 
					last_t = (time_high_ovfs << 24) + ((time_high + time_low_ovfs) << 12) + time_low; 
					if (first_run){
						first_t = last_t; 
						first_run = 0; 
					}
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
				case EVT3_CONTINUED_4:
					break; 

				default:
					MEAS_EVENT_TYPE_NOT_RECOGNISED(event_type, cargo); 
			}
		}
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = dim; 
	if (values_read==0)
		cargo->events_info.finished = 1;
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
	size_t values_read=0, j=0, i=0, dim=cargo->events_info.dim; 
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
	while (i < dim && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < dim && j < values_read; j++){
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
				case EVT3_CONTINUED_4:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
		byte_pt += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 

	cargo->events_info.start_byte = byte_pt; 
	cargo->events_info.dim = i; 
	if (values_read==0)
		cargo->events_info.finished = 1; 
	return 0; 
}

DLLEXPORT int save_evt3(const char* fpath, event_t* arr, evt3_cargo_t* cargo, size_t buff_size){
	char header[150]; 
	sprintf(header, "%c This EVT3 file has been generated through expelliarmus (https://github.com/fabhertz95/expelliarmus.git) %c%c evt 3.0 %c", (char)HEADER_START, (char)HEADER_END, (char)HEADER_START, (char)HEADER_END); 
	const size_t header_len = strlen(header); 
	FILE* fp; 
	if (cargo->events_info.start_byte == 0){
		fp = fopen(fpath, "wb");	
		CHECK_FILE(fp, fpath); 
		CHECK_FWRITE((cargo->events_info.start_byte = fwrite(header, sizeof(char), header_len, fp)), header_len); 
	} else {
		fp = fopen(fpath, "ab"); 
		CHECK_FILE(fp, fpath); 
	}

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff); 
	
	// Indices to read the file.
	size_t j=0, i=0, dim=cargo->events_info.dim; 

	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU; 
	// Temporary values to handle overflows.
	uint8_t evt_high=0, evt_low=0, evt_y=0, write_event=0, first_event=1; 
	uint16_t time_high=0, time_low=0, time_high_prev=0, time_low_prev=0; 
	
	// Reading the file.
	while (i < dim){
		// First event.
		if (first_event){
			// Y address.
			buff[0] = ((uint16_t) EVT3_EVT_ADDR_Y) << 12; 
			buff[0] |= ((uint16_t) arr[0].y) & mask_11b; 
			// Time low.
			buff[1] = ((uint16_t) EVT3_TIME_HIGH) << 12; 
			buff[1] |= (uint16_t) ((arr[0].t >> 12) & mask_12b); 
			// Time high.
			buff[2] = ((uint16_t) EVT3_TIME_LOW) << 12; 
			buff[2] |= (uint16_t) (arr[0].t & mask_12b); 
			// X address and polarity.
			buff[3] = ((uint16_t) EVT3_EVT_ADDR_X) << 12; 
			buff[3] |= ((uint16_t) arr[0].p) << 11; 
			buff[3] |= ((uint16_t) arr[0].x) & mask_11b; 
			i = 1; 
			CHECK_FWRITE(fwrite(buff, sizeof(*buff), 4, fp), 4); 
			first_event = 0; 
		}
		for (j=0; i < dim && j < buff_size; j++){
			if (write_event){
				write_event = 0; 
				buff[j] = ((uint16_t) EVT3_EVT_ADDR_X) << 12; 
				buff[j] |= ((uint16_t) arr[i].p & 1U) << 11; 
				buff[j] |= (uint16_t) arr[i++].x & mask_11b; 
			} else if (!evt_y && arr[i].y != arr[i-1].y){
				evt_y = 1; 
				buff[j] = ((uint16_t) EVT3_EVT_ADDR_Y) << 12; 
				buff[j] |= (uint16_t) arr[i].y & mask_11b; 
			} else {
				time_high = ((uint16_t) (arr[i].t >> 32)) & mask_12b; 
				time_high_prev = ((uint16_t) (arr[i-1].t >> 32)) & mask_12b; 
				time_low = (uint16_t) (arr[i].t & mask_12b); 
				time_low_prev = (uint16_t) (arr[i-1].t & mask_12b); 
				if (!evt_high && time_high != time_high_prev){
					evt_high = 1; 
					// Writing the event to file.
					buff[j] = ((uint16_t) EVT3_TIME_HIGH) << 12; 
					buff[j] |= time_high; 
				} else if (!evt_low && time_low != time_low_prev){
					evt_low = 1; 
					buff[j] = ((uint16_t) EVT3_TIME_LOW) << 12; 
					buff[j] |= time_low; 
				} else {
					evt_y = evt_high = evt_low = 0; 
					j--; 
					write_event = 1; 
				}
			}
		}
		// Writing buffer to file.
		CHECK_FWRITE(fwrite(buff, sizeof(*buff), j, fp), j); 
	}
	fclose(fp); 
	free(buff); 
	return 0; 
}

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
				case EVT3_CONTINUED_4:
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
