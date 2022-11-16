#include "muggle.h"
#include <stdio.h>
#include <stdlib.h>

DLLEXPORT void read_dat_chunk(const char* fpath, size_t buff_size, dat_chunk_wrap_t* chunk_wrap, size_t nevents_per_chunk){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	event_array_t arr; 
	arr.dim = 0; 
	arr.allocated_space = DEFAULT_ARRAY_DIM; 
	chunk_wrap->arr = arr; 

	if (chunk_wrap->bytes_read == 0){
		// Jumping over the headers.
		uint8_t pt; 
		do {
			do { 
				chunk_wrap->bytes_read += fread(&pt, 1, 1, fp); 
			} while (pt != 0x0A); 
			chunk_wrap->bytes_read += fread(&pt, 1, 1, fp); 
			if (pt != 0x25) break; 
		} while (1); 

		fseek(fp, 1, SEEK_CUR); 
		chunk_wrap->bytes_read++; 
	} else {
		int err = fseek(fp, (long)(chunk_wrap->bytes_read), SEEK_SET); 
		if (err != 0){
			chunk_wrap->bytes_read = 0; 
			return; 
		}
	}

	// Now we can start to have some fun.
	// Allocating the array of events.
	// Timestamp.
	arr.t_arr = (timestamp_t*) malloc(arr.allocated_space * sizeof(timestamp_t));
	CHECK_ALLOCATION(arr.t_arr); 
	// X coordinate.
	arr.x_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	CHECK_ALLOCATION(arr.x_arr); 
	// Y coordinate.
	arr.y_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	CHECK_ALLOCATION(arr.y_arr); 
	// Polarity.
	arr.p_arr = (polarity_t*) malloc(arr.allocated_space * sizeof(polarity_t));
	CHECK_ALLOCATION(arr.p_arr); 
	
	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t));
	CHECK_ALLOCATION(buff); 
	
	// Temporary event data structure.
	event_t event_tmp; event_tmp.x=0; event_tmp.y=0; event_tmp.t=0; event_tmp.p=0;  
	size_t values_read=0, j=0, i=0; 
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	uint64_t time_ovfs=0, timestamp=0; 
	while (i < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < nevents_per_chunk && j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)(buff[j])) < timestamp) // Overflow.
				time_ovfs++; 
			timestamp = (time_ovfs<<32) | ((uint64_t)buff[j]);
			CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t); 
			event_tmp.t = (timestamp_t) timestamp; 
			// Event polarity.
			event_tmp.p = (polarity_t) ((buff[j+1] >> 28) & mask_4b); 
			// Event y address.
			event_tmp.y = (pixel_t) ((buff[j+1] >> 14) & mask_14b); 
			// Event x address. 
			event_tmp.x = (pixel_t) (buff[j+1] & mask_14b); 
			append_event(&event_tmp, &arr, i++); 
		}
		chunk_wrap->bytes_read += j*sizeof(*buff); 
	}
	free(buff); 
	fclose(fp); 
	// Reallocating to save memory.
	event_array_t arr_tmp = arr; 
	// Timestamp.
	arr_tmp.t_arr = (timestamp_t*) realloc(arr_tmp.t_arr, i * sizeof(timestamp_t));
	CHECK_ALLOCATION(arr_tmp.t_arr); 
	arr.t_arr = arr_tmp.t_arr; 
	// X coordinate.
	arr_tmp.x_arr = (pixel_t*) realloc(arr_tmp.x_arr, i * sizeof(pixel_t));
	CHECK_ALLOCATION(arr_tmp.x_arr); 
	arr.x_arr = arr_tmp.x_arr; 
	// Y coordinate.
	arr_tmp.y_arr = (pixel_t*) realloc(arr_tmp.y_arr, i * sizeof(pixel_t));
	CHECK_ALLOCATION(arr_tmp.y_arr); 
	arr.y_arr = arr_tmp.y_arr; 
	// Polarity.
	arr_tmp.p_arr = (polarity_t*) realloc(arr_tmp.p_arr, i * sizeof(polarity_t));
	CHECK_ALLOCATION(arr_tmp.p_arr); 
	arr.p_arr = arr_tmp.p_arr; 
	arr.dim = i; 
	arr.allocated_space = i; 

	chunk_wrap->arr = arr; 
	return; 
}

DLLEXPORT void read_evt2_chunk(const char* fpath, size_t buff_size, evt2_chunk_wrap_t* chunk_wrap, size_t nevents_per_chunk){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	event_array_t arr; 
	arr.dim = 0; 
	arr.allocated_space = DEFAULT_ARRAY_DIM; 
	chunk_wrap->arr = arr; 

	if (chunk_wrap->bytes_read == 0){
		chunk_wrap->time_high = 0; 

		// Jumping over the headers.
		uint8_t pt; 
		do {
			do { 
				chunk_wrap->bytes_read += fread(&pt, 1, 1, fp); 
			} while (pt != 0x0A); 
			chunk_wrap->bytes_read += fread(&pt, 1, 1, fp); 
			if (pt != 0x25) break; 
		} while (1); 

		// Coming back to previous byte.
		fseek(fp, -1, SEEK_CUR); 
		chunk_wrap->bytes_read--;
	} else {
		int err = fseek(fp, (long)(chunk_wrap->bytes_read), SEEK_SET); 
		if (err != 0){
			chunk_wrap->bytes_read = 0; 
			return; 
		}
	}

	// Allocating the array of events.
	// Timestamp.
	arr.t_arr = (timestamp_t*) malloc(arr.allocated_space * sizeof(timestamp_t));
	CHECK_ALLOCATION(arr.t_arr); 
	// X coordinate.
	arr.x_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	CHECK_ALLOCATION(arr.x_arr); 
	// Y coordinate.
	arr.y_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	CHECK_ALLOCATION(arr.y_arr); 
	// Polarity.
	arr.p_arr = (polarity_t*) malloc(arr.allocated_space * sizeof(polarity_t));
	CHECK_ALLOCATION(arr.p_arr); 

	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_ALLOCATION(buff); 

	event_t event_tmp; event_tmp.t=0; event_tmp.p=0; event_tmp.x=0; event_tmp.y=0;   

	uint8_t event_type; 
	size_t i=0, j=0, values_read=0; 
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	uint64_t time_low=0, timestamp=0; 
	while (i < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < nevents_per_chunk && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					event_tmp.p = (polarity_t) event_type; 
					// Getting 6LSBs of the time stamp. 
					time_low = ((uint64_t)((buff[j] >> 22) & mask_6b)); 
					timestamp = (chunk_wrap->time_high << 6) | time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t);
					event_tmp.t = (timestamp_t) timestamp; 
					// Getting event addresses.
					event_tmp.x = (pixel_t) ((buff[j] >> 11) & mask_11b); 
					event_tmp.y = (pixel_t) (buff[j] & mask_11b); 
					append_event(&event_tmp, &arr, i++); 
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					chunk_wrap->time_high = (uint64_t)(buff[j] & mask_28b); 
					break; 

				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					fprintf(stderr, "Error: event type not valid: 0x%x.\n", event_type); 
					exit(EXIT_FAILURE); 
			}
		}
		chunk_wrap->bytes_read += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	// Reallocating to save memory.
	event_array_t arr_tmp = arr; 
	// Timestamp.
	arr_tmp.t_arr = (timestamp_t*) realloc(arr_tmp.t_arr, i * sizeof(timestamp_t));
	CHECK_ALLOCATION(arr_tmp.t_arr); 
	arr.t_arr = arr_tmp.t_arr; 
	// X coordinate.
	arr_tmp.x_arr = (pixel_t*) realloc(arr_tmp.x_arr, i * sizeof(pixel_t));
	CHECK_ALLOCATION(arr_tmp.x_arr); 
	arr.x_arr = arr_tmp.x_arr; 
	// Y coordinate.
	arr_tmp.y_arr = (pixel_t*) realloc(arr_tmp.y_arr, i * sizeof(pixel_t));
	CHECK_ALLOCATION(arr_tmp.y_arr); 
	arr.y_arr = arr_tmp.y_arr; 
	// Polarity.
	arr_tmp.p_arr = (polarity_t*) realloc(arr_tmp.p_arr, i * sizeof(polarity_t));
	CHECK_ALLOCATION(arr_tmp.p_arr); 
	arr.p_arr = arr_tmp.p_arr; 
	arr.dim = i; 
	arr.allocated_space = i; 
	
	chunk_wrap->arr = arr; 
	return; 
}

DLLEXPORT void read_evt3_chunk(const char* fpath, size_t buff_size, evt3_chunk_wrap_t* chunk_wrap, size_t nevents_per_chunk){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	event_array_t arr; 
	arr.dim = 0; arr.allocated_space = DEFAULT_ARRAY_DIM; 
	chunk_wrap->arr = arr; 

	if (chunk_wrap->bytes_read == 0){
		chunk_wrap->base_x = 0; 
		chunk_wrap->time_high = 0; 
		chunk_wrap->time_low = 0; 
		chunk_wrap->time_high_ovfs = 0; 
		chunk_wrap->time_low_ovfs = 0; 
		chunk_wrap->event_tmp.t=0; chunk_wrap->event_tmp.x=0; chunk_wrap->event_tmp.y=0; chunk_wrap->event_tmp.p=0; 

		// Jumping over the headers.
		uint8_t pt; 
		 do {
			do { 
				chunk_wrap->bytes_read += fread(&pt, 1, 1, fp); 
			} while (pt != 0x0A); 
			chunk_wrap->bytes_read += fread(&pt, 1, 1, fp); 
			if (pt != 0x25) break; 
		} while (1); 
		
		// Coming back to previous byte.
		fseek(fp, -1, SEEK_CUR); 
		chunk_wrap->bytes_read--; 
	} else {
		int err = fseek(fp, (long)(chunk_wrap->bytes_read), SEEK_SET); 
		if (err != 0){
			chunk_wrap->bytes_read = 0; 
			return;
		}
	}

	// Allocating the array of events.
	// Timestamp.
	arr.t_arr = (timestamp_t*) malloc(arr.allocated_space * sizeof(timestamp_t));
	CHECK_ALLOCATION(arr.t_arr); 
	// X coordinate.
	arr.x_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	CHECK_ALLOCATION(arr.x_arr); 
	// Y coordinate.
	arr.y_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	CHECK_ALLOCATION(arr.y_arr); 
	// Polarity.
	arr.p_arr = (polarity_t*) malloc(arr.allocated_space * sizeof(polarity_t));
	CHECK_ALLOCATION(arr.p_arr); 

	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_ALLOCATION(buff); 

	size_t values_read=0, j=0, i=0; 
	uint8_t event_type; 
	uint16_t k=0, num_vect_events=0; 
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	uint64_t buff_tmp=0, timestamp=0; 
	while (i < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < nevents_per_chunk && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					chunk_wrap->event_tmp.y = (pixel_t)(buff[j] & mask_11b);
					break; 

				case EVT3_EVT_ADDR_X:
					chunk_wrap->event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					chunk_wrap->event_tmp.x = (pixel_t)(buff[j] & mask_11b);
					append_event(&(chunk_wrap->event_tmp), &arr, i++); 
					break; 

				case EVT3_VECT_BASE_X:
					chunk_wrap->event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					chunk_wrap->base_x = (uint16_t)(buff[j] & mask_11b);
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
						if (buff_tmp%2){
							chunk_wrap->event_tmp.x = (pixel_t)(chunk_wrap->base_x + k); 
							append_event(&(chunk_wrap->event_tmp), &arr, i++); 
						}
						buff_tmp = buff_tmp >> 1; 
					}
					chunk_wrap->base_x += num_vect_events; 
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < chunk_wrap->time_low) // Overflow.
						chunk_wrap->time_low_ovfs++; 
					chunk_wrap->time_low = buff_tmp; 
					timestamp = (chunk_wrap->time_high_ovfs<<24) + ((chunk_wrap->time_high + chunk_wrap->time_low_ovfs)<<12) + chunk_wrap->time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, chunk_wrap->event_tmp.t)
					chunk_wrap->event_tmp.t = (timestamp_t) timestamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < chunk_wrap->time_high) // Overflow.
						chunk_wrap->time_high_ovfs++; 
					chunk_wrap->time_high = buff_tmp; 
					timestamp = (chunk_wrap->time_high_ovfs<<24) + ((chunk_wrap->time_high + chunk_wrap->time_low_ovfs)<<12) + chunk_wrap->time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, chunk_wrap->event_tmp.t)
					chunk_wrap->event_tmp.t = (timestamp_t) timestamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					fprintf(stderr, "Error: event type not valid: 0x%x.\n", event_type); 
					exit(EXIT_FAILURE); 
			}
		}
		chunk_wrap->bytes_read += j*sizeof(*buff);
	}
	fclose(fp); 
	free(buff); 
	// Reallocating to save memory.
	event_array_t arr_tmp = arr; 
	// Timestamp.
	arr_tmp.t_arr = (timestamp_t*) realloc(arr_tmp.t_arr, i * sizeof(timestamp_t));
	CHECK_ALLOCATION(arr_tmp.t_arr); 
	arr.t_arr = arr_tmp.t_arr; 
	// X coordinate.
	arr_tmp.x_arr = (pixel_t*) realloc(arr_tmp.x_arr, i * sizeof(pixel_t));
	CHECK_ALLOCATION(arr_tmp.x_arr); 
	arr.x_arr = arr_tmp.x_arr; 
	// Y coordinate.
	arr_tmp.y_arr = (pixel_t*) realloc(arr_tmp.y_arr, i * sizeof(pixel_t));
	CHECK_ALLOCATION(arr_tmp.y_arr); 
	arr.y_arr = arr_tmp.y_arr; 
	// Polarity.
	arr_tmp.p_arr = (polarity_t*) realloc(arr_tmp.p_arr, i * sizeof(polarity_t));
	CHECK_ALLOCATION(arr_tmp.p_arr); 
	arr.p_arr = arr_tmp.p_arr; 
	arr.dim = i; 
	arr.allocated_space = i; 

	chunk_wrap->arr = arr; 
	return; 
}


