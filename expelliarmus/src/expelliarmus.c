#include "expelliarmus.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CHECK_ALLOCATION(pt) {\
	if (pt == NULL){\
		fprintf(stderr, "Error during dinamic array memory allocation.\n");\
		exit(1);\
	}\
}

#define CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "Error while opening the file \"%s\".\n", fpath);\
		exit(2);\
	}\
}

void append_event(const event_t* event, event_array_t* arr, size_t i){
	event_array_t arr_tmp = *arr; 
	if (i >= arr->allocated_space){
	  	// Doubling the storage. 
		// Timestamp.
		arr_tmp.t_arr = (timestamp_t*) realloc(arr_tmp.t_arr, 2*(arr->allocated_space)*sizeof(timestamp_t));
		CHECK_ALLOCATION(arr_tmp.t_arr); 
		arr->t_arr = arr_tmp.t_arr; 
		// X coordinate.
		arr_tmp.x_arr = (pixel_t*) realloc(arr_tmp.x_arr, 2*(arr->allocated_space)*sizeof(pixel_t));
		CHECK_ALLOCATION(arr_tmp.x_arr); 
		arr->x_arr = arr_tmp.x_arr; 
		// Y coordinate.
		arr_tmp.y_arr = (pixel_t*) realloc(arr_tmp.y_arr, 2*(arr->allocated_space)*sizeof(pixel_t));
		CHECK_ALLOCATION(arr_tmp.y_arr); 
		arr->y_arr = arr_tmp.y_arr; 
		// Polarity.
		arr_tmp.p_arr = (polarity_t*) realloc(arr_tmp.p_arr, 2*(arr->allocated_space)*sizeof(polarity_t));
		CHECK_ALLOCATION(arr_tmp.p_arr); 
		arr->p_arr = arr_tmp.p_arr; 
		arr->allocated_space *= 2; 
	}
	arr->t_arr[i] = event->t; 
	arr->x_arr[i] = event->x; 
	arr->y_arr[i] = event->y; 
	arr->p_arr[i] = event->p; 
	return; 
}

/*
 * Functions for reading events to arrays.
 */

DLLEXPORT event_array_t read_dat(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp); 
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp); 
		if (pt != 0x25) break; 
	} while (1); 

	fseek(fp, 1, SEEK_CUR); 

	// Now we can start to have some fun.
	event_array_t arr; 
	arr.dim = 0; arr.allocated_space = DEFAULT_ARRAY_DIM; 
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
	uint32_t* buff = (uint32_t*) malloc(2*buff_size * sizeof(uint32_t));
	CHECK_ALLOCATION(buff); 
	
	// Temporary event data structure.
	struct event_s event_tmp; event_tmp.x=0; event_tmp.y=0; event_tmp.t=0; event_tmp.p=0;  
	size_t values_read=0, j=0, i=0; 
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	uint64_t time_ovfs=0, time_val=0; 
	while ((values_read = fread(buff, sizeof(buff[0]), 2*buff_size, fp)) > 0){
		for (j=0; j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)buff[j]) < time_val) // Overflow.
				time_ovfs++; 
			time_val = (uint64_t) buff[j]; 
			event_tmp.t = (timestamp_t)((time_ovfs<<32) | time_val); 
			// Event polarity.
			event_tmp.p = (polarity_t) ((buff[j+1] >> 28) & mask_4b); 
			// Event y address.
			event_tmp.y = (pixel_t) ((buff[j+1] >> 14) & mask_14b); 
			// Event x address. 
			event_tmp.x = (pixel_t) (buff[j+1] & mask_14b); 
			append_event(&event_tmp, &arr, i++); 
		}
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
	return arr; 
}	

DLLEXPORT event_array_t read_evt2(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp); 
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp); 
		if (pt != 0x25) break; 
	} while (1); 
	
	// Coming back to previous byte.
	fseek(fp, -1, SEEK_CUR); 

	// Preparing the data structure. 
	event_array_t arr; 
	arr.dim = 0; arr.allocated_space = DEFAULT_ARRAY_DIM; 
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
	uint8_t event_type; 
	struct event_s event_tmp; event_tmp.t=0; event_tmp.p=0; event_tmp.x=0; event_tmp.y=0;   
	size_t i=0, j=0, values_read=0; 
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	uint32_t time_high=0, time_low=0; 
	while ((values_read = fread(buff, sizeof(buff[0]), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					event_tmp.p = (polarity_t) event_type; 
					// Getting 6LSBs of the time stamp. 
					time_low = ((uint32_t)((buff[j] >> 22) & mask_6b)); 
					event_tmp.t = (timestamp_t)((time_high << 6) | time_low); 
					// Getting event addresses.
					event_tmp.x = (pixel_t) ((buff[j] >> 11) & mask_11b); 
					event_tmp.y = (pixel_t) (buff[j] & mask_11b); 
					append_event(&event_tmp, &arr, i++); 
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					time_high = (uint32_t)(buff[j] & mask_28b); 
					break; 

				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					fprintf(stderr, "Error: event type not valid: 0x%x 0x%x.\n", event_type, EVT2_CD_ON); 
					exit(1); 
			}
		}
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
	return arr; 
}

DLLEXPORT event_array_t read_evt3(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp); 
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp); 
		if (pt != 0x25) break; 
	} while (1); 
	
	// Coming back to previous byte.
	fseek(fp, -1, SEEK_CUR); 

	// Preparing the data structure. 
	event_array_t arr; 
	arr.dim = 0; arr.allocated_space = DEFAULT_ARRAY_DIM; 
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
	event_t event_tmp; event_tmp.t=0; event_tmp.x=0; event_tmp.y=0; event_tmp.p=0;  

	uint16_t base_x=0, k=0, num_vect_events=0; 
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	uint64_t buff_tmp=0, time_high=0, time_low=0, time_stamp=0, time_high_ovfs=0, time_low_ovfs=0; 
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
			// Getting the event type. 
			event_type = (buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					event_tmp.y = (pixel_t)(buff[j] & mask_11b);
					break; 

				case EVT3_EVT_ADDR_X:
					event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					event_tmp.x = (pixel_t)(buff[j] & mask_11b);
					append_event(&event_tmp, &arr, i++); 
					break; 

				case EVT3_VECT_BASE_X:
					event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					base_x = (uint16_t)(buff[j] & mask_11b);
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
						if (buff_tmp%2){
							event_tmp.x = (pixel_t)(base_x + k); 
							append_event(&event_tmp, &arr, i++); 
						}
						buff_tmp = buff_tmp >> 1; 
					}
					base_x += num_vect_events; 
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < time_low) // Overflow.
						time_low_ovfs++; 
					time_low = buff_tmp; 
					time_stamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					event_tmp.t = (timestamp_t) time_stamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < time_high) // Overflow.
						time_high_ovfs++; 
					time_high = buff_tmp; 
					time_stamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					event_tmp.t = (timestamp_t) time_stamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					fprintf(stderr, "Error: event type not valid: 0x%x peppa.\n", event_type); 
					exit(1); 
			}
		}
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
	return arr; 
}

/*
 * Functions for cutting DAT and RAW files to a certain number of events.
 */

DLLEXPORT size_t cut_dat(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "wb"); 
	CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp_in); 
			fwrite(&pt, 1, 1, fp_out);  
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp_in); 
		if (pt != 0x25) break; 
		fwrite(&pt, 1, 1, fp_out);  
	} while (1); 

	fwrite(&pt, 1, 1, fp_out);  
	fread(&pt, 1, 1, fp_in); 
	fwrite(&pt, 1, 1, fp_out);  
	
	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(2*buff_size * sizeof(uint32_t));
	CHECK_ALLOCATION(buff); 
	
	// Temporary event data structure.
	size_t values_read=0, j=0, i=0; 
	uint64_t time_ovfs=0, timestamp=0, first_timestamp=0; 
	new_duration *= 1000; // Converting to microseconds.
	while ((timestamp-first_timestamp) < (uint64_t)new_duration && (values_read = fread(buff, sizeof(*buff), 2*buff_size, fp_in)) > 0){
		for (j=0; (timestamp-first_timestamp) < (uint64_t)new_duration && j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)buff[j]) < timestamp) // Overflow.
				time_ovfs++; 
			timestamp = (uint64_t) buff[j]; 
			if (i++ == 0)
				first_timestamp = timestamp; 
			fwrite(&buff[j], sizeof(*buff), 2, fp_out);
		}
	}
	free(buff); 
	fclose(fp_in); 
	fclose(fp_out); 
	return i; 
}

DLLEXPORT size_t cut_evt2(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "wb"); 
	CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp_in); 
			fwrite(&pt, 1, 1, fp_out);
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp_in); 
		if (pt != 0x25) break; 
		fwrite(&pt, 1, 1, fp_out); 
	} while (1); 
	
	// Coming back to previous byte.
	fseek(fp_in, -1, SEEK_CUR); 

	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_ALLOCATION(buff); 
	uint8_t event_type; 
	size_t i=0, j=0, values_read=0; 
	uint64_t first_timestamp=0, timestamp=0, time_high=0, time_low=0;
	const uint32_t mask_28b = 0xFFFFFFFU, mask_6b=0x3FU; 
	new_duration *= 1000; // Converting to microseconds.
	while ((timestamp-first_timestamp) < (uint64_t)new_duration && (values_read = fread(buff, sizeof(*buff), buff_size, fp_in)) > 0){
		for (j=0; (timestamp-first_timestamp) < (uint64_t)new_duration && j<values_read; j++){
			fwrite(&buff[j], sizeof(*buff), 1, fp_out); 
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					// Getting 6LSBs of the time stamp. 
					time_low = ((uint64_t)((buff[j] >> 22) & mask_6b)); 
					timestamp = ((time_high << 6) | time_low); 
					if (i++ == 0)
						first_timestamp = timestamp;
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					time_high = (uint64_t)(buff[j] & mask_28b); 
					break; 
					break; 

				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					fprintf(stderr, "Error: event type not valid: 0x%x 0x%x.\n", event_type, EVT2_CD_ON); 
					exit(1); 
			}
		}
	}
	fclose(fp_out); 
	fclose(fp_in); 
	free(buff); 
	return i; 
}

DLLEXPORT size_t cut_evt3(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "w+b"); 
	CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp_in); 
			fwrite(&pt, 1, 1, fp_out); 
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp_in); 
		if (pt != 0x25) break; 
		fwrite(&pt, 1, 1, fp_out); 
	} while (1); 
	
	// Coming back to previous byte.
	fseek(fp_in, -1, SEEK_CUR); 

	size_t i = 0; 

	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_ALLOCATION(buff); 
	size_t values_read=0, j=0; 

	uint64_t buff_tmp=0, k=0, num_vect_events=0; 
	const uint16_t mask_8b = 0xFFU, mask_12b = 0xFFFU; 
	uint8_t event_type=0, recording_finished=0, last_events_acquired=0, get_out=0; 
	uint64_t first_timestamp=0, timestamp=0, time_high=0, time_high_ovfs=0, time_low=0, time_low_ovfs=0; 
	// Converting duration to microseconds.
	new_duration *= 1000; 

	while (!get_out && (values_read = fread(buff, sizeof(*buff), buff_size, fp_in)) > 0){
		for (j=0; !get_out && j<values_read; j++){
			fwrite(&buff[j], sizeof(*buff), 1, fp_out); 
			// Getting the event type. 
			event_type = (buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					break; 

				case EVT3_EVT_ADDR_X:
					if (recording_finished)
						last_events_acquired = 1; 
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
						last_events_acquired = 1; 
					break; 

				case EVT3_TIME_LOW:
					if ((timestamp - first_timestamp) >= (uint64_t)new_duration){
						recording_finished = 1;
						if (last_events_acquired)
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
						if (last_events_acquired)
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
					fprintf(stderr, "Error: event type not valid: 0x%x.\n", event_type); 
					exit(1); 
			}
		}
	}
	fclose(fp_in); 
	fclose(fp_out); 
	free(buff); 
	return i; 
}
