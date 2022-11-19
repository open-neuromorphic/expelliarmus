#include "events.h"
#include "wizard.h"
#include "muggle.h"
#include <stdio.h>
#include <stdlib.h>

#define CHECK_FILE(fp, fpath, chunk, void_chunk){\
	if (fp==NULL){\
		fprintf(stderr, "ERROR: the file \"%s\" could not be opened.\n", fpath);\
		free_event_array(&(chunk->arr));\
		*chunk = void_chunk(); \
		return; \
	}\
}

#define CHECK_FILE_END(chunk, void_chunk){\
	if (chunk->bytes_read >= chunk->file_size){\
		fprintf(stderr, "ERROR: The file is finished.\n");\
		free_event_array(&(chunk->arr));\
		*chunk = void_chunk();\
		return;\
	}\
}

#define CHECK_CHUNK_ALLOCATION(chunk, void_chunk){\
	if (is_void_event_array(&(chunk->arr))){\
		fprintf(stderr, "ERROR: The event array allocation has failed.\n");\
		*chunk = void_chunk();\
		return;\
	}\
}

#define CHECK_BUFF_ALLOCATION(buff, chunk, void_chunk){\
	if (buff==NULL){\
		fprintf(stderr, "ERROR: the buffer used to read the binary file could not be allocated.\n");\
		free_event_array(&(chunk->arr));\
		*chunk = void_chunk();\
		return;\
	}\
}

#define CHECK_JUMP_HEADER(chunk, void_chunk){\
	if (chunk->bytes_read == 0){\
		fprintf(stderr, "ERROR: jump_header failed.\n");\
		free_event_array(&(chunk->arr));\
		*chunk = void_chunk();\
		return;\
	}\
}

#define CHECK_FSEEK(fn, chunk, void_chunk){\
	if (fn!=0){\
		fprintf(stderr, "ERROR: fseek failed.\n");\
		free_event_array(&(chunk->arr));\
		*chunk = void_chunk();\
		return;\
	}\
}

#define CHECK_ADD_EVENT(chunk, void_chunk){\
	if (is_void_event_array(&(chunk->arr))){\
		fprintf(stderr, "ERROR: failed to add event to array (failed memory reallocation).\n");\
		*chunk = void_chunk();\
		return;\
	}\
}

#define EVENT_TYPE_NOT_RECOGNIZED(event_type, chunk, void_chunk){\
	fprintf(stderr, "ERROR: The event type 0x%x has not been recognized.\n", event_type);\
	free_event_array(&(chunk->arr));\
	*chunk = void_chunk();\
   	return;\
}

dat_chunk_t void_dat_chunk(){
	return (dat_chunk_t){void_event_array(), 0, 0}; 
}

evt2_chunk_t void_evt2_chunk(){
	return (evt2_chunk_t){void_event_array(), 0, 0, 0}; 
}

evt3_chunk_t void_evt3_chunk(){
	return (evt3_chunk_t){void_event_array(), 0, 0, 0, 0, 0, 0, 0, void_event()}; 
}

DLLEXPORT void read_dat_chunk(const char* fpath, size_t buff_size, dat_chunk_t* chunk, size_t nevents_per_chunk){
	CHECK_FILE_END(chunk, void_dat_chunk); 
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath, chunk, void_dat_chunk); 

	if (chunk->bytes_read == 0){
		// Chunk initialization.
		*chunk = void_dat_chunk(); 
		// Jumping over the headers.
		chunk->bytes_read = jump_header(fp, NULL, 0U); 
		CHECK_JUMP_HEADER(chunk, void_dat_chunk);
		CHECK_FSEEK(fseek(fp, 1, SEEK_CUR), chunk, void_dat_chunk); 
		chunk->bytes_read++; 
	} else
		CHECK_FSEEK(fseek(fp, (long)(chunk->bytes_read), SEEK_SET), chunk, void_dat_chunk);

	chunk->arr = malloc_event_array(DEFAULT_ARRAY_DIM);
	CHECK_CHUNK_ALLOCATION(chunk, void_dat_chunk);

	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t));
	CHECK_BUFF_ALLOCATION(buff, chunk, void_dat_chunk); 
	
	// Temporary event data structure.
	event_t event_tmp = void_event();
	// Indices to access the binary file.
	size_t values_read=0, j=0, i=0; 
	// Masks to extract bits.
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	// Values for overflow checking.
	uint64_t time_ovfs=0, timestamp=0; 
	// Reading the file.
	while (i < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < nevents_per_chunk && j<values_read; j+=2){
			// Event timestamp.
			if (((uint64_t)(buff[j])) < timestamp) // Overflow.
				time_ovfs++; 
			timestamp = (time_ovfs<<32) | ((uint64_t)buff[j]);
			CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t); 
			event_tmp.t = (timestamp_t) timestamp; 
			// Event x address. 
			event_tmp.x = (pixel_t) (buff[j+1] & mask_14b); 
			// Event polarity.
			event_tmp.p = (polarity_t) ((buff[j+1] >> 28) & mask_4b); 
			// Event y address.
			event_tmp.y = (pixel_t) ((buff[j+1] >> 14) & mask_14b); 
			add_event(&event_tmp, &(chunk->arr), i++); 
			CHECK_ADD_EVENT(chunk, void_dat_chunk); 
		}
		chunk->bytes_read += j*sizeof(*buff); 
	}
	free(buff); 
	fclose(fp); 
	// Reallocating to save memory.
	chunk->arr = realloc_event_array(&(chunk->arr), chunk->arr.dim); 
	CHECK_CHUNK_ALLOCATION(chunk, void_dat_chunk);
	return; 
}

DLLEXPORT void read_evt2_chunk(const char* fpath, size_t buff_size, evt2_chunk_t* chunk, size_t nevents_per_chunk){
	CHECK_FILE_END(chunk, void_evt2_chunk);
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath, chunk, void_evt2_chunk); 

	if (chunk->bytes_read == 0){
		*chunk = void_evt2_chunk(); 
		// Jumping over the headers.
		chunk->bytes_read = jump_header(fp, NULL, 0U); 
		CHECK_JUMP_HEADER(chunk, void_evt2_chunk); 
		// Coming back to previous byte.
		CHECK_FSEEK(fseek(fp, -1, SEEK_CUR), chunk, void_evt2_chunk);
		chunk->bytes_read--;
	} else 
		CHECK_FSEEK(fseek(fp, (long)(chunk->bytes_read), SEEK_SET), chunk, void_evt2_chunk); 
	
	chunk->arr = malloc_event_array(DEFAULT_ARRAY_DIM); 
	CHECK_CHUNK_ALLOCATION(chunk, void_evt2_chunk); 

	// Buffer to read the binary file.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_BUFF_ALLOCATION(buff, chunk, void_evt2_chunk); 

	// Temporary event data structure.
	event_t event_tmp = void_event();

	// Byte to detect event_type.
	uint8_t event_type; 
	// Indices to read the file.
	size_t i=0, j=0, values_read=0; 
	// Masks to extract bits.
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	// Temporary variables for overflow detection.
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
					timestamp = (chunk->time_high << 6) | time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, event_tmp.t);
					event_tmp.t = (timestamp_t) timestamp; 
					// Getting event addresses.
					event_tmp.x = (pixel_t) ((buff[j] >> 11) & mask_11b); 
					event_tmp.y = (pixel_t) (buff[j] & mask_11b); 
					add_event(&event_tmp, &(chunk->arr), i++); 
					CHECK_ADD_EVENT(chunk, void_evt2_chunk); 
					break; 

				case EVT2_TIME_HIGH:
					// Adding 28 MSBs to timestamp.
					chunk->time_high = (uint64_t)(buff[j] & mask_28b); 
					break; 

				case EVT2_EXT_TRIGGER:
				case EVT2_OTHERS:
				case EVT2_CONTINUED:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNIZED(event_type, chunk, void_evt2_chunk); 
			}
		}
		chunk->bytes_read += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	chunk->arr = realloc_event_array(&(chunk->arr), chunk->arr.dim); 
	CHECK_CHUNK_ALLOCATION(chunk, void_evt2_chunk); 
	return; 
}

DLLEXPORT void read_evt3_chunk(const char* fpath, size_t buff_size, evt3_chunk_t* chunk, size_t nevents_per_chunk){
	CHECK_FILE_END(chunk, void_evt3_chunk); 
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath, chunk, void_evt3_chunk); 

	if (chunk->bytes_read == 0){
		*chunk = void_evt3_chunk(); 
		// Jumping over the headers.
		chunk->bytes_read = jump_header(fp, NULL, 0U); 	
		// Coming back to previous byte.
		CHECK_FSEEK(fseek(fp, -1, SEEK_CUR), chunk, void_evt3_chunk); 
		chunk->bytes_read--; 
	} else
		CHECK_FSEEK(fseek(fp, (long)(chunk->bytes_read), SEEK_SET), chunk, void_evt3_chunk); 

	chunk->arr = malloc_event_array(DEFAULT_ARRAY_DIM); 
	CHECK_CHUNK_ALLOCATION(chunk, void_evt3_chunk); 

	// Buffer to read the binary file.
	uint16_t* buff = (uint16_t*) malloc(buff_size * sizeof(uint16_t)); 
	CHECK_BUFF_ALLOCATION(buff, chunk, void_evt3_chunk); 

	// Indices to access the binary file.
	size_t values_read=0, j=0, i=0; 
	// Byte to detect the event type.
	uint8_t event_type; 
	// Variables for treating the vectorized events.
	uint16_t k=0, num_vect_events=0; 
	// Masks to extract bits.
	const uint16_t mask_11b=0x7FFU, mask_12b=0xFFFU, mask_8b=0xFFU; 
	// Temporary variables to handle overflow.
	uint64_t buff_tmp=0, timestamp=0; 
	while (i < nevents_per_chunk && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < nevents_per_chunk && j<values_read; j++){
			// Getting the event type. 
			event_type = (uint8_t)(buff[j] >> 12); 
			switch (event_type){
				case EVT3_EVT_ADDR_Y:
					chunk->event_tmp.y = (pixel_t)(buff[j] & mask_11b);
					break; 

				case EVT3_EVT_ADDR_X:
					chunk->event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					chunk->event_tmp.x = (pixel_t)(buff[j] & mask_11b);
					add_event(&(chunk->event_tmp), &(chunk->arr), i++); 
					CHECK_ADD_EVENT(chunk, void_evt3_chunk);
					break; 

				case EVT3_VECT_BASE_X:
					chunk->event_tmp.p = (polarity_t) ((buff[j] >> 11)%2); 
					chunk->base_x = (uint16_t)(buff[j] & mask_11b);
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
							chunk->event_tmp.x = (pixel_t)(chunk->base_x + k); 
							add_event(&(chunk->event_tmp), &(chunk->arr), i++); 
							CHECK_ADD_EVENT(chunk, void_evt3_chunk);
						}
						buff_tmp = buff_tmp >> 1; 
					}
					chunk->base_x += num_vect_events; 
					num_vect_events = 0; 
					break; 

				case EVT3_TIME_LOW:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < chunk->time_low) // Overflow.
						chunk->time_low_ovfs++; 
					chunk->time_low = buff_tmp; 
					timestamp = (chunk->time_high_ovfs<<24) + ((chunk->time_high + chunk->time_low_ovfs)<<12) + chunk->time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, chunk->event_tmp.t)
					chunk->event_tmp.t = (timestamp_t) timestamp; 
					break; 

				case EVT3_TIME_HIGH:
					buff_tmp = (uint64_t)(buff[j] & mask_12b);
					if (buff_tmp < chunk->time_high) // Overflow.
						chunk->time_high_ovfs++; 
					chunk->time_high = buff_tmp; 
					timestamp = (chunk->time_high_ovfs<<24) + ((chunk->time_high + chunk->time_low_ovfs)<<12) + chunk->time_low;
					CHECK_TIMESTAMP_MONOTONICITY(timestamp, chunk->event_tmp.t)
					chunk->event_tmp.t = (timestamp_t) timestamp; 
					break; 

				case EVT3_EXT_TRIGGER:
				case EVT3_OTHERS:
				case EVT3_CONTINUED_12:
					break; 

				default:
					EVENT_TYPE_NOT_RECOGNIZED(event_type, chunk, void_evt3_chunk); 
			}
		}
		chunk->bytes_read += j*sizeof(*buff);
	}
	fclose(fp); 
	free(buff); 
	// Reallocating to save memory.
	chunk->arr = realloc_event_array(&(chunk->arr), chunk->arr.dim); 
	CHECK_CHUNK_ALLOCATION(chunk, void_evt3_chunk); 
	return; 
}


