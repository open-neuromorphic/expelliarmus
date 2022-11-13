#include "muggle.h"
#include <stdio.h>
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

DLLEXPORT dat_chunk_wrap_t read_dat_chunk(const char* fpath, size_t buff_size, size_t bytes_read, size_t nevents_per_chunk){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	event_array_t arr; 
	arr.dim = 0; 
	arr.allocated_space = DEFAULT_ARRAY_DIM; 
	dat_chunk_wrap_t ret; 
	ret.arr = arr; 
	ret.bytes_read = 0; 

	if (bytes_read == 0){
		// Jumping over the headers.
		uint8_t pt; 
		do {
			do { 
				bytes_read += fread(&pt, 1, 1, fp); 
			} while (pt != 0x0A); 
			bytes_read += fread(&pt, 1, 1, fp); 
			if (pt != 0x25) break; 
		} while (1); 

		fseek(fp, 1, SEEK_CUR); 
		bytes_read++; 
	} else {
		int err = fseek(fp, (long)(bytes_read), SEEK_SET); 
		if (err != 0)
			return ret;
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
	uint32_t* buff = (uint32_t*) malloc(2*buff_size * sizeof(uint32_t));
	CHECK_ALLOCATION(buff); 
	
	// Temporary event data structure.
	event_t event_tmp; event_tmp.x=0; event_tmp.y=0; event_tmp.t=0; event_tmp.p=0;  
	size_t values_read=0, j=0, i=0; 
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	uint64_t time_ovfs=0, time_val=0; 
	size_t nevents=0; 
	while (nevents < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), 2*buff_size, fp)) > 0){
		for (j=0; nevents < nevents_per_chunk && j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)(buff[j])) < time_val) // Overflow.
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
			nevents++; 
		}
		bytes_read += j*sizeof(*buff); 
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

	ret.arr = arr; 
	ret.bytes_read = bytes_read; 
	fprintf(stderr, "%lu\n", ret.bytes_read);  
	return ret; 
}

/*
DLLEXPORT event_array_t read_evt2_chunk(const char* fpath, size_t* dim, size_t buff_size, size_t* bytes_read, size_t nevents_per_chunk, event_t* old_time_high){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	size_t bytes_read_i = *bytes_read; 
	if (bytes_read_i == 0){
		// Jumping over the headers.
		uint8_t pt; 
		do {
			do { 
				bytes_read_i += fread(&pt, 1, 1, fp); 
			} while (pt != 0x0A); 
			bytes_read_i += fread(&pt, 1, 1, fp); 
			if (pt != 0x25) break; 
		} while (1); 

		// Coming back to previous byte.
		fseek(fp, -1, SEEK_CUR); 
		bytes_read_i--;
	} else {
		int err = fseek(fp, (long)(bytes_read_i), SEEK_SET); 
		event_t time_high = *old_time_high; 
		if (err != 0){
			*bytes_read = 0; 
			return NULL; 
		}
	}

	// Preparing the data structure. 
	size_t allocated_space = DEFAULT_ARRAY_DIM; 
	event_array_t arr= (event_array_t) malloc(allocated_space * sizeof(event_t));
   	CHECK_ALLOCATION(arr); 

	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_ALLOCATION(buff); 
	uint8_t event_type; 
	struct event_s event_tmp; event_tmp.t=0; event_tmp.p=0; event_tmp.x=0; event_tmp.y=0;   
	size_t i=0, j=0, values_read=0; 
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	event_t time_low=0; 
	size_t nevents=0; 
	while (nevents < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; nevents < nevents_per_chunk && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					event_tmp.p = (event_t) event_type; 
					// Getting 6LSBs of the time stamp. 
					time_low = ((event_t)((buff[j] >> 22) & mask_6b)); 
					event_tmp.t = ((time_high << 6) | time_low); 
					// Getting event addresses.
					event_tmp.x = (event_t) ((buff[j] >> 11) & mask_11b); 
					event_tmp.y = (event_t) (buff[j] & mask_11b); 
					append_event(&event_tmp, &arr, &allocated_space, &i); 
					nevents++; 
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					time_high = (event_t)(buff[j] & mask_28b); 
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
		bytes_read_i += j*sizeof(buff[0]); 
	}
	fclose(fp); 
	free(buff); 
	// Reallocating to save memory.
	event_array_t tmp = realloc(arr, i*sizeof(event_t)); 
	CHECK_ALLOCATION(tmp); 
	arr= tmp; 
	*dim = i; 
	*bytes_read = bytes_read_i; 
	return arr; 
}

DLLEXPORT event_array_t read_evt3_chunk(const char* fpath, size_t* dim, size_t buff_size, size_t* bytes_read, size_t nevents_per_chunk){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	size_t bytes_read_i = *bytes_read; 
	if (bytes_read_i == 0){
		// Jumping over the headers.
		uint8_t pt; 
		do {
			do { 
				fread(&pt, 1, 1, fp); 
				bytes_read_i++; 
			} while (pt != 0x0A); 
			fread(&pt, 1, 1, fp); 
			bytes_read_i++; 
			if (pt != 0x25) break; 
		} while (1); 

		// Coming back to previous byte.
		fseek(fp, -1, SEEK_CUR); 
		bytes_read_i--; 
	} else {
		int err = fseek(fp, bytes_read_i, SEEK_SET); 
		if (err != 0){
			*bytes_read = 0; 
			return NULL; 
		}
	}

	// Preparing the data structure. 
	size_t allocated_space = DEFAULT_ARRAY_DIM; 
	size_t i = 0; 
	event_array_t arr = (event_array_t) malloc(allocated_space * sizeof(event_t));
   	CHECK_ALLOCATION(arr); 

	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_ALLOCATION(buff); 
	size_t values_read=0, j=0; 
	uint8_t event_type; 
	struct event_s event_tmp; event_tmp.t=0; 

	event_t buff_tmp=0, base_x=0, k=0, num_vect_events=0; 
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	event_t time_high=0, time_low=0, time_stamp=0, time_high_ovfs=0, time_low_ovfs=0; 
	size_t nevents=0; 
	while (nevents < nevents_per_chunk && (values_read = fread(buff, sizeof(buff[0]), buff_size, fp)) > 0){
		for (j=0; nevents < nevents_per_chunk && j<values_read; j++){
			bytes_read_i += sizeof(*buff); 
			// Getting the event type. 
			event_type = (buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					event_tmp.y = (event_t)(buff[j] & mask_11b);
					break; 

				case EVT3_EVT_ADDR_X:
					event_tmp.p = (event_t) (buff[j] >> 11)%2; 
					event_tmp.x = (event_t)(buff[j] & mask_11b);
					append_event(&event_tmp, &arr, &allocated_space, &i); 
					nevents++; 
					break; 

				case EVT3_VECT_BASE_X:
					event_tmp.p = (event_t) (buff[j+1] >> 11)%2; 
					base_x = (event_t)(buff[j] & mask_11b);
					break; 

				case EVT3_VECT_12:
					num_vect_events = 12; 
					buff_tmp = (event_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events == 0){
						num_vect_events = 8; 
						buff_tmp = (event_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events; k++){
						if (buff_tmp%2){
							event_tmp.x = base_x + k; 
							append_event(&event_tmp, &arr, &allocated_space, &i); 
							nevents++; 
						}
						buff_tmp = buff_tmp >> 1; 
					}
					base_x += num_vect_events; 
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (event_t)(buff[j] & mask_12b);
					if (buff_tmp < time_low) // Overflow.
						time_low_ovfs++; 
					time_low = buff_tmp; 
					time_stamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					event_tmp.t = time_stamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (event_t)(buff[j] & mask_12b);
					if (buff_tmp < time_high) // Overflow.
						time_high_ovfs++; 
					time_high = buff_tmp; 
					time_stamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					event_tmp.t = time_stamp; 
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
	event_array_t tmp = realloc(arr, i*sizeof(event_t)); 
	CHECK_ALLOCATION(tmp); 
	arr= tmp; 
	*dim = i; 
	*bytes_read = bytes_read_i; 
	return arr; 
}
*/ 

