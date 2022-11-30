#include "wizard.h"
#include "events.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define CHECK_FILE(fp, fpath){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the input file \"%s\" could not be opened.\n", fpath); \
		return -1;\
	}\
}

#define CHECK_JUMP_HEADER(fn){\
	if (fn == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		return -1;\
	}\
}

#define CHECK_FSEEK(fn){\
	if (fn != 0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		return -1;\
	}\
}

#define CHECK_BUFF_ALLOCATION(buff){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the input file could not be allocated.\n"); \
		return -1;\
	}\
}

#define EVENT_TYPE_NOT_RECOGNISED(event_type){\
	fprintf(stderr, "ERROR: event type not recognised: 0x%x.\n", event_type);\
	return -1;\
}\

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
}\

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

DLLEXPORT size_t measure_dat(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U)); 
	// Jumping a byte.
	CHECK_FSEEK(fseek(fp, 1, SEEK_CUR)); 

	// Buffer to read binary data.
	uint64_t* buff = (uint64_t*) malloc(buff_size * sizeof(uint64_t));
	CHECK_BUFF_ALLOCATION(buff); 
	
	size_t dim=0, values_read=0; 
	// Reading the file.
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0)
		dim += values_read; 
	free(buff); 
	fclose(fp); 
	return dim;
}

DLLEXPORT int read_dat(const char* fpath, event_t* arr, dat_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	if (cargo->events_info.bytes_read == 0){
		// Jumping over the headers.
		CHECK_JUMP_HEADER((cargo->events_info.bytes_read=jump_header(fp, NULL, 0U))); 
		// Jumping a byte.
		CHECK_FSEEK(fseek(fp, 1, SEEK_CUR)); 
		cargo->events_info.bytes_read++; 
	} else
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.bytes_read, SEEK_SET)); 

	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t));
	CHECK_BUFF_ALLOCATION(buff); 

	// Indices to keep track of how many items are read from the file.
	size_t values_read=0, j=0, i=0; 
	timestamp_t timestamp=0; 
	// Masks to extract bits.
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	// Reading the file.
	while (i < cargo->events_info.dim && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < cargo->events_info.dim && j<values_read; j+=2, i++){
			// Event timestamp.
			if (((uint64_t)buff[j]) < ((uint64_t)cargo->last_t)) // Overflow.
				cargo->time_ovfs++; 
			timestamp = (timestamp_t)((cargo->time_ovfs<<32) | ((uint64_t)buff[j])); 
			CHECK_TIMESTAMP_MONOTONICITY(timestamp, cargo->last_t);

			arr[i].t = timestamp; 
			cargo->last_t = timestamp; 
			// Event x address. 
			arr[i].x = (address_t) (buff[j+1] & mask_14b); 
			// Event y address.
			arr[i].y = (address_t) ((buff[j+1] >> 14) & mask_14b); 
			// Event polarity.
			arr[i].p = (polarity_t) ((buff[j+1] >> 28) & mask_4b); 
		}
		cargo->events_info.bytes_read += j*sizeof(*buff); 
	}
	free(buff); 
	fclose(fp); 
	cargo->events_info.dim = i; 
	if (i==0)
		return -1; 
	return 0; 
}	

DLLEXPORT size_t measure_evt2(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U)); 	
	// Coming back to previous byte.
	CHECK_FSEEK(fseek(fp, -1, SEEK_CUR)); 

	// Buffer to read the file.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_BUFF_ALLOCATION(buff); 

	// The byte that identifies the event type.
	uint8_t event_type; 
	// Indices to access the input file.
	size_t j=0, values_read=0, dim=0; 
	// Reading the file.
	while ((values_read = fread(buff, sizeof(buff[0]), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					dim++; 
					break; 

				case EVT2_TIME_HIGH:
				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
	}
	fclose(fp); 
	free(buff); 
	return dim; 
}

DLLEXPORT int read_evt2(const char* fpath, event_t* arr, evt2_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	if (cargo->events_info.bytes_read == 0){
		// Jumping over the headers.
		CHECK_JUMP_HEADER((cargo->events_info.bytes_read=jump_header(fp, NULL, 0U))); 	
		// Coming back to previous byte.
		CHECK_FSEEK(fseek(fp, -1, SEEK_CUR)); 
		cargo->events_info.bytes_read--; 
	} else
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.bytes_read, SEEK_SET)); 

	// Buffer to read the file.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_BUFF_ALLOCATION(buff); 

	// The byte that identifies the event type.
	uint8_t event_type; 
	// Indices to access the input file.
	size_t i=0, j=0, values_read=0; 
	// Masks to extract bits.
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	// Values to handle overflows.
	uint64_t time_low=0;
	timestamp_t timestamp=0; 
	// Reading the file.
	while (i < cargo->events_info.dim && (values_read = fread(buff, sizeof(buff[0]), buff_size, fp)) > 0){
		for (j=0; i < cargo->events_info.dim && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t) (buff[j] >> 28); 
			switch (event_type){
				case EVT2_CD_ON:
				case EVT2_CD_OFF:
					// Getting 6LSBs of the time stamp. 
					time_low = ((uint64_t)((buff[j] >> 22) & mask_6b)); 
					timestamp = (timestamp_t)((cargo->time_high << 6) | time_low);
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, cargo->last_t);
					arr[i].t = timestamp; 
					cargo->last_t = timestamp; 
					// Getting event addresses.
					arr[i].x = (address_t) ((buff[j] >> 11) & mask_11b); 
					arr[i].y = (address_t) (buff[j] & mask_11b); 
					// Getting event polarity.
					arr[i++].p = (polarity_t) event_type; 
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					cargo->time_high = (uint64_t)(buff[j] & mask_28b); 
					break; 

				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
		cargo->events_info.bytes_read += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = i;
	if (i==0)
		return -1; 
	return 0; 
}

DLLEXPORT size_t measure_evt3(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U)); 
	
	// Coming back to previous byte.
	CHECK_FSEEK(fseek(fp, -1, SEEK_CUR)); 

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff); 
	
	// Indices to read the file.
	size_t values_read=0, j=0, dim=0; 
	// Byte that identifies the event type.
	uint8_t event_type; 

	// Counters used to keep track of number of events_info encoded in vectors and of the base x address of these.
	uint16_t k=0, num_vect_events_info=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary values to handle overflows.
	uint64_t buff_tmp=0;
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
					num_vect_events_info = 12; 
					buff_tmp = (uint16_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events_info == 0){
						num_vect_events_info = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events_info; k++){
						if (buff_tmp & (1U<<k)){
							dim++; 
						}
					}
					num_vect_events_info = 0; 
					break; 

				case EVT3_TIME_LOW:
				case EVT3_TIME_HIGH:
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNISED(event_type); 
			}
		}
	}
	fclose(fp); 
	free(buff); 
	return dim; 
}

DLLEXPORT int read_evt3(const char* fpath, event_t* arr, evt3_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	if (cargo->events_info.bytes_read == 0){
		// Jumping over the headers.
		CHECK_JUMP_HEADER((cargo->events_info.bytes_read=jump_header(fp, NULL, 0U))); 
		// Coming back to previous byte.
		CHECK_FSEEK(fseek(fp, -1, SEEK_CUR)); 
		cargo->events_info.bytes_read--; 
	} else 
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.bytes_read, SEEK_SET)); 

	// Buffer used to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff); 
	
	// Indices to read the file.
	size_t values_read=0, j=0, i=0; 
	// Byte that identifies the event type.
	uint8_t event_type; 

	// Counters used to keep track of number of events_info encoded in vectors and of the base x address of these.
	uint16_t k=0, num_vect_events_info=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary values to handle overflows.
	uint64_t buff_tmp=0;
   	timestamp_t timestamp=0; 
	// Reading the file.
	while (i < cargo->events_info.dim && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < cargo->events_info.dim && j < values_read; j++){
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
					num_vect_events_info = 12; 
					buff_tmp = (uint16_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events_info == 0){
						num_vect_events_info = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events_info; k++){
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
					cargo->base_x += num_vect_events_info; 
					num_vect_events_info = 0; 
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
		cargo->events_info.bytes_read += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = i; 
	if (i==0)
		return -1; 
	return 0; 
}

/*
 * Functions for cutting DAT and RAW files to a certain number of events_info.
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

	// Temporary values to keep track of vectorized events_info.
	uint64_t buff_tmp=0, k=0, num_vect_events_info=0; 
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
					num_vect_events_info = 12; 
					buff_tmp = (uint64_t)(buff[j] & mask_12b);

				case EVT3_VECT_8:
					if (num_vect_events_info == 0){
						num_vect_events_info = 8; 
						buff_tmp = (uint64_t)(buff[j] & mask_8b);
					}
					for (k=0; k<num_vect_events_info; k++){
						if (buff_tmp%2)
							i++; 
						buff_tmp = buff_tmp >> 1; 
					}
					num_vect_events_info = 0; 
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
