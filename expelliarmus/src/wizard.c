#include "wizard.h"
#include "events.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CHECK_FILE(fp, fpath, arr){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n", fpath); \
		free_event_array(&arr);\
		return void_event_array();\
	}\
}

#define CHECK_BUFF_ALLOCATION(buff, arr){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n"); \
		free_event_array(&arr);\
		return void_event_array();\
	}\
}

#define CHECK_EVENT_ARRAY_ALLOC(arr){\
	if (is_void_event_array(&arr)){\
		fprintf(stderr, "ERROR: the event array could not be allocated in memory.\n");\
		return arr;\
	}\
}

#define CHECK_ADD_EVENT(arr){\
	if (is_void_event_array(&arr)){\
		fprintf(stderr, "ERROR: the event could no be added to the array (failed memory reallocation).\n");\
		return arr;\
	}\
}

#define CHECK_EVENT_ARRAY_SHRINK(arr){\
	if (is_void_event_array(&arr)){\
		fprintf(stderr, "ERROR: the event array memory space could not be shrinked (failed memory reallocation).\n");\
		return arr;\
	}\
}

#define EVENT_TYPE_NOT_RECOGNISED(event_type, arr){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	free_event_array(&arr);\
	return void_event_array();\
}\

#define CUT_CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n", fpath);\
		return 0;\
	}\
}

#define CUT_CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n");\
		return 0;\
	}\
}

#define CHECK_FWRITE(fn, expected){\
	if (expected != fn){\
		fprintf(stderr, "ERROR: fwrite failed.\n");\
		return 0;\
	}\
}

#define CHECK_FSEEK(fn, arr){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		free_event_array(&arr);\
		return void_event_array();\
	}\
}

#define CUT_CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return 0;\
	}\
}

#define CUT_EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return 0;\
}\

#define CHECK_JUMP_HEADER(fn, arr){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		free_event_array(&arr);\
		return void_event_array();\
	}\
}

#define CUT_CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return 0;\
	}\
}

size_t jump_header(FILE* fp_in, FILE* fp_out, uint8_t copy_file){
	size_t bytes_read = 0; 
	uint8_t c; 
	 do {
	   	do { 
			bytes_read += fread(&c, 1, 1, fp_in); 
			if (copy_file)
				CHECK_FWRITE(fwrite(&c, 1, 1, fp_out), 1);  
		} while (c != 0x0A); 
		bytes_read += fread(&c, 1, 1, fp_in); 
		if (c != 0x25) break; 
		if (copy_file)
			CHECK_FWRITE(fwrite(&c, 1, 1, fp_out), 1);  
	} while (1); 
	return bytes_read; 
}

DLLEXPORT event_array_t read_dat(const char* fpath, size_t buff_size){
	// Allocating the array.
	event_array_t arr = malloc_event_array(DEFAULT_ARRAY_DIM); 
	CHECK_EVENT_ARRAY_ALLOC(arr); 

	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath, arr); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U), arr); 
	// Jumping a byte.
	CHECK_FSEEK(fseek(fp, 1, SEEK_CUR), arr); 

	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t));
	CHECK_BUFF_ALLOCATION(buff, arr); 
	
	// Temporary event data structure.
	event_t event_tmp = void_event();
	// Indices to keep track of how many items are read from the file.
	size_t values_read=0, j=0, i=0; 
	// Masks to extract bits.
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	// Values to keep track of overflows.
	uint64_t time_ovfs=0, timestamp=0; 
	// Reading the file.
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)buff[j]) < timestamp) // Overflow.
				time_ovfs++; 
			timestamp = (time_ovfs<<32) | ((uint64_t)buff[j]); 
			CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t);
			event_tmp.t = (timestamp_t) timestamp; 
			// Event x address. 
			event_tmp.x = (pixel_t) (buff[j+1] & mask_14b); 
			// Event y address.
			event_tmp.y = (pixel_t) ((buff[j+1] >> 14) & mask_14b); 
			// Event polarity.
			event_tmp.p = (polarity_t) ((buff[j+1] >> 28) & mask_4b); 
			add_event(&event_tmp, &arr, i++); 
			CHECK_ADD_EVENT(arr); 
		}
	}
	free(buff); 
	fclose(fp); 
	// Reallocating the array to save space.
	arr = realloc_event_array(&arr, arr.dim); 	
	CHECK_EVENT_ARRAY_SHRINK(arr); 
	return arr; 
}	

DLLEXPORT event_array_t read_evt2(const char* fpath, size_t buff_size){
	// Allocating the event array.
	event_array_t arr = malloc_event_array(DEFAULT_ARRAY_DIM); 
	CHECK_EVENT_ARRAY_ALLOC(arr); 

	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath, arr); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U), arr); 	
	// Coming back to previous byte.
	CHECK_FSEEK(fseek(fp, -1, SEEK_CUR), arr); 

	// Buffer to read the file.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_BUFF_ALLOCATION(buff, arr); 

	// The byte that identifies the event type.
	uint8_t event_type; 
	// Temporary event data structure.
	event_t event_tmp = void_event();
	// Indices to access the input file.
	size_t i=0, j=0, values_read=0; 
	// Masks to extract bits.
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	// Values to handle overflows.
	uint64_t time_high=0, time_low=0, timestamp=0; 
	// Reading the file.
	while ((values_read = fread(buff, sizeof(buff[0]), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					event_tmp.p = (polarity_t) event_type; 
					// Getting 6LSBs of the time stamp. 
					time_low = ((uint64_t)((buff[j] >> 22) & mask_6b)); 
					timestamp = (time_high << 6) | time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t);
					event_tmp.t = (timestamp_t) timestamp; 
					// Getting event addresses.
					event_tmp.x = (pixel_t) ((buff[j] >> 11) & mask_11b); 
					event_tmp.y = (pixel_t) (buff[j] & mask_11b); 
					add_event(&event_tmp, &arr, i++); 
					CHECK_ADD_EVENT(arr); 
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					time_high = (uint64_t)(buff[j] & mask_28b); 
					break; 

				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type, arr); 
			}
		}
	}
	fclose(fp); 
	free(buff); 
	arr = realloc_event_array(&arr, arr.dim); 
	CHECK_EVENT_ARRAY_SHRINK(arr); 
	return arr; 
}

DLLEXPORT event_array_t read_evt3(const char* fpath, size_t buff_size){
	// Allocating the event array.
	event_array_t arr = malloc_event_array(DEFAULT_ARRAY_DIM); 
	CHECK_EVENT_ARRAY_ALLOC(arr); 

	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath, arr); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U), arr); 
	
	// Coming back to previous byte.
	CHECK_FSEEK(fseek(fp, -1, SEEK_CUR), arr); 

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff, arr); 
	// Indices to read the file.
	size_t values_read=0, j=0, i=0; 
	// Byte that identifies the event type.
	uint8_t event_type; 

	// Temporary event data structure.
	event_t event_tmp = void_event();

	// Counters used to keep track of number of events encoded in vectors and of the base x address of these.
	uint16_t base_x=0, k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary values to handle overflows.
	uint64_t buff_tmp=0, time_high=0, time_low=0, timestamp=0, time_high_ovfs=0, time_low_ovfs=0; 
	// Reading the file.
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					event_tmp.y = (pixel_t)(buff[j] & mask_11b);
					break; 

				case EVT3_EVT_ADDR_X:
					event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					event_tmp.x = (pixel_t)(buff[j] & mask_11b);
					add_event(&event_tmp, &arr, i++); 
					CHECK_ADD_EVENT(arr); 
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
							add_event(&event_tmp, &arr, i++); 
							CHECK_ADD_EVENT(arr); 
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
					timestamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t);
					event_tmp.t = (timestamp_t) timestamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < time_high) // Overflow.
						time_high_ovfs++; 
					time_high = buff_tmp; 
					timestamp = (time_high_ovfs<<24) + ((time_high+time_low_ovfs)<<12) + time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t);
					event_tmp.t = (timestamp_t) timestamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type, arr); 
			}
		}
	}
	fclose(fp); 
	free(buff); 
	arr = realloc_event_array(&arr, arr.dim); 
	CHECK_EVENT_ARRAY_SHRINK(arr); 
	return arr; 
}

/*
 * Functions for cutting DAT and RAW files to a certain number of events.
 */

DLLEXPORT size_t cut_dat(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CUT_CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "wb"); 
	CUT_CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	uint8_t c; 
	CUT_CHECK_JUMP_HEADER(jump_header(fp_in, fp_out, 1U)); 
	CHECK_FWRITE(fwrite(&c, 1, 1, fp_out), 1);  
	fread(&c, 1, 1, fp_in); 
	CHECK_FWRITE(fwrite(&c, 1, 1, fp_out), 1);  
	
	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t));
	CUT_CHECK_BUFF_ALLOCATION(buff); 
	
	// Indices to read the file.
	size_t values_read=0, j=0, i=0; 
	// Values to keep track of overflows and of first timestamp encountered.
	uint64_t time_ovfs=0, timestamp=0, first_timestamp=0; 
	// Converting duration from milliseconds to microseconds.
	new_duration *= 1000; 
	while ((timestamp-first_timestamp) < (uint64_t)new_duration && (values_read = fread(buff, sizeof(*buff), buff_size, fp_in)) > 0){
		for (j=0; (timestamp-first_timestamp) < (uint64_t)new_duration && j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)buff[j]) < timestamp) // Overflow.
				time_ovfs++; 
			timestamp = (uint64_t) buff[j]; 
			if (i++ == 0)
				first_timestamp = timestamp; 
		}
		CHECK_FWRITE(fwrite(buff, sizeof(*buff), j, fp_out), j); 
	}
	free(buff); 
	fclose(fp_in); 
	fclose(fp_out); 
	return i; 
}

DLLEXPORT size_t cut_evt2(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CUT_CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "wb"); 
	CUT_CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	CUT_CHECK_JUMP_HEADER(jump_header(fp_in, fp_out, 1U)); 	
	// Coming back to previous byte.
	CUT_CHECK_FSEEK(fseek(fp_in, -1, SEEK_CUR)); 

	// Buffer to read the binary file.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CUT_CHECK_BUFF_ALLOCATION(buff); 

	// Byte to check the event type.
	uint8_t event_type; 
	// Indices to read the file.
	size_t i=0, j=0, values_read=0; 
	// Values to keep track of first timestamp and overflows.
	uint64_t first_timestamp=0, timestamp=0, time_high=0, time_low=0;
	// Masks to extract bits.
	const uint32_t mask_28b = 0xFFFFFFFU, mask_6b=0x3FU; 
	// Converting duration from milliseconds to microseconds.
	new_duration *= 1000;
	while ((timestamp-first_timestamp) < (uint64_t)new_duration && (values_read = fread(buff, sizeof(*buff), buff_size, fp_in)) > 0){
		for (j=0; (timestamp-first_timestamp) < (uint64_t)new_duration && j<values_read; j++){
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
					CUT_EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
		CHECK_FWRITE(fwrite(buff, sizeof(*buff), j, fp_out), j); 
	}
	fclose(fp_out); 
	fclose(fp_in); 
	free(buff); 
	return i; 
}

DLLEXPORT size_t cut_evt3(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CUT_CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "w+b"); 
	CUT_CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	CUT_CHECK_JUMP_HEADER(jump_header(fp_in, fp_out, 1U)); 	
	// Coming back to previous byte.
	CUT_CHECK_FSEEK(fseek(fp_in, -1, SEEK_CUR)); 

	// Buffer to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CUT_CHECK_BUFF_ALLOCATION(buff); 

	// Indices to access the binary file.
	size_t values_read=0, j=0, i=0; 

	// Temporary values to keep track of vectorized events.
	uint64_t buff_tmp=0, k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_8b = 0xFFU, mask_12b = 0xFFFU; 
	// Flags to recognise event type and end of file.
	uint8_t event_type=0, recording_finished=0, last_events_acquired=0, get_out=0; 
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
