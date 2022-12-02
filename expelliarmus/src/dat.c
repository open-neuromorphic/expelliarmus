#include "dat.h"
#include <stdio.h>

DLLEXPORT size_t measure_dat(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U)); 
	// Jumping a byte.
	CHECK_FSEEK(fseek(fp, 2, SEEK_CUR)); 

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

	if (cargo->events_info.bytes_done == 0){
		// Jumping over the headers.
		CHECK_JUMP_HEADER((cargo->events_info.bytes_done=jump_header(fp, NULL, 0U))); 
		CHECK_FSEEK(fseek(fp, 2, SEEK_CUR)); 
		cargo->events_info.bytes_done += 2; 
	} else
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.bytes_done, SEEK_SET)); 

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
		cargo->events_info.bytes_done += j*sizeof(*buff); 
	}
	free(buff); 
	fclose(fp); 
	cargo->events_info.dim = i; 
	if (i==0)
		return -1; 
	return 0; 
}	

DLLEXPORT size_t cut_dat(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CUT_CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "wb"); 
	CUT_CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	uint8_t c[2]; 
	CUT_CHECK_JUMP_HEADER(jump_header(fp_in, fp_out, 1U)); 
	fread(c, 1, 2, fp_in); 
	CHECK_FWRITE(fwrite(c, 1, 2, fp_out), 2);  
	
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

