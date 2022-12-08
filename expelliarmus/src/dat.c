#include "dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define LOOP_CONDITION(window, last_t, ovfs, first_t) (window > (((ovfs << 32) | last_t) - first_t))

DLLEXPORT void measure_dat(const char* fpath, dat_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	MEAS_CHECK_FILE(fp, fpath, cargo); 
	
	// Jumping over the headers.
	if (cargo->events_info.start_byte == 0){
		MEAS_CHECK_JUMP_HEADER((cargo->events_info.start_byte = jump_header(fp, NULL, 0U)), cargo);
		// Jumping two bytes.
		MEAS_CHECK_FSEEK(fseek(fp, 2, SEEK_CUR), cargo); 
		cargo->events_info.start_byte += 2; 
	} else {
		MEAS_CHECK_FSEEK(fseek(fp, (long)cargo->events_info.start_byte, SEEK_SET), cargo); 
	}

	// Buffer to read binary data.
	uint64_t* buff = (uint64_t*) malloc(buff_size * sizeof(uint64_t));
	MEAS_CHECK_BUFF_ALLOCATION(buff, cargo); 
	
	size_t dim=0, values_read=0, j=0; 
	
	// Reading the file.
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0)
		dim += values_read; 

	free(buff); 
	fclose(fp); 
	cargo->events_info.dim = dim; 
	if (values_read==0)
		cargo->events_info.finished = 1;
	return;
}

DLLEXPORT void get_time_window_dat(const char* fpath, dat_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	MEAS_CHECK_FILE(fp, fpath, cargo); 
	
	// Jumping over the headers.
	if (cargo->events_info.start_byte == 0){
		MEAS_CHECK_JUMP_HEADER((cargo->events_info.start_byte = jump_header(fp, NULL, 0U)), cargo);
		// Jumping two bytes.
		MEAS_CHECK_FSEEK(fseek(fp, 2, SEEK_CUR), cargo); 
		cargo->events_info.start_byte += 2; 
	} else {
		MEAS_CHECK_FSEEK(fseek(fp, (long)cargo->events_info.start_byte, SEEK_SET), cargo); 
	}

	// Buffer to read binary data.
	uint64_t* buff = (uint64_t*) malloc(buff_size * sizeof(uint64_t));
	MEAS_CHECK_BUFF_ALLOCATION(buff, cargo); 
	
	size_t dim=0, values_read=0, j=0; 
	uint64_t last_t = 0, time_ovfs = cargo->time_ovfs; 	
	uint64_t first_t = 0;
	uint64_t time_window = (uint64_t) cargo->events_info.time_window, t=0; 
	uint8_t first_run=1, is_time_window = cargo->events_info.is_time_window; 
	const uint64_t mask_32b=0xFFFFFFFFU;
	
	// Reading the file.
	while (LOOP_CONDITION(time_window, last_t, time_ovfs, first_t) && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; LOOP_CONDITION(time_window, last_t, time_ovfs, first_t) && j < values_read; j++){
			t = buff[j] & mask_32b; 
			if (t < last_t)
				time_ovfs++; 
			last_t = t; 
			if (first_run){
				first_t = (time_ovfs << 32) | last_t; 
				first_run = 0; 
			}
		}
		dim += j; 
	}
	free(buff); 
	fclose(fp); 
	cargo->events_info.dim = dim; 
	if (values_read==0)
		cargo->events_info.finished = 1;
	return;
}

DLLEXPORT int read_dat(const char* fpath, event_t* arr, dat_cargo_t* cargo, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	if (cargo->events_info.start_byte == 0){
		CHECK_JUMP_HEADER((cargo->events_info.start_byte = jump_header(fp, NULL, 0U))); 
		CHECK_FSEEK(fseek(fp, 2, SEEK_CUR));
		cargo->events_info.start_byte += 2;
	} else {
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.start_byte, SEEK_SET)); 
	}
	size_t byte_pt = cargo->events_info.start_byte; 

	// Buffer to read binary data.
	uint64_t* buff = (uint64_t*) malloc(buff_size * sizeof(uint64_t));
	CHECK_BUFF_ALLOCATION(buff); 

	// Indices to keep track of how many items are read from the file.
	size_t values_read=0, j=0, i=0, dim=cargo->events_info.dim; 
	timestamp_t timestamp=0; 
	// Masks to extract bits.
	const uint64_t mask_4b=0xFU, mask_14b=0x3FFFU, mask_32b=0xFFFFFFFFU;
	uint64_t lower=0, upper=0; 
	
	// Reading the file.
	while (i < dim && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; i < dim && j < values_read; j++){
			// Event timestamp.
			lower = buff[j] & mask_32b; 
			upper = buff[j] >> 32; 
			if (lower < cargo->last_t) // Overflow.
				cargo->time_ovfs++; 
			timestamp = (timestamp_t)((cargo->time_ovfs<<32) | lower); 
			CHECK_TIMESTAMP_MONOTONICITY(timestamp, ((cargo->time_ovfs << 32) | cargo->last_t));

			arr[i].t = timestamp; 
			cargo->last_t = lower; 
			// Event x address. 
			arr[i].x = (address_t) (upper & mask_14b); 
			// Event y address.
			arr[i].y = (address_t) ((upper >> 14) & mask_14b); 
			// Event polarity.
			arr[i++].p = (polarity_t) ((upper >> 28) & mask_4b); 
		}
		byte_pt += j*sizeof(*buff); 
	}
	free(buff); 
	fclose(fp); 

	cargo->events_info.start_byte = byte_pt; 
	cargo->events_info.dim = i; 
	if (values_read==0)
		cargo->events_info.finished = 1;
	return 0; 
}	

DLLEXPORT int save_dat(const char* fpath, event_t* arr, dat_cargo_t* cargo, size_t buff_size){
	char header[200]; 
	sprintf(header, "%c This DAT file has been generated through expelliarmus (https://github.com/fabhertz95/expelliarmus.git) %c%c Data file containing CD events %c%c Version 2 %c", (char)HEADER_START, (char)HEADER_END, (char)HEADER_START, (char)HEADER_END, (char)HEADER_START, (char)HEADER_END); 
	const size_t header_len = strlen(header); 
	FILE* fp; 
	if (cargo->events_info.start_byte == 0){
		fp = fopen(fpath, "wb");	
		CHECK_FILE(fp, fpath); 
		CHECK_FWRITE(fwrite(header, sizeof(char), header_len, fp), header_len); 
		// Writing Event2D type and event size as header information.
		const uint8_t header_info[2] = {0x0, 0x8}; 
		CHECK_FWRITE(fwrite(header_info, sizeof(*header_info), 2, fp), 2); 
		cargo->events_info.start_byte = header_len + 2*sizeof(*header_info); 
	} else {
		fp = fopen(fpath, "ab"); 
		CHECK_FILE(fp, fpath); 
	}

	// Buffer to read the file.
	uint64_t* buff = (uint64_t*) malloc(buff_size * sizeof(uint64_t)); 
	CHECK_BUFF_ALLOCATION(buff); 

	// Indices to access the input file.
	size_t i=0, j=0; 
	// Masks to extract bits.
	const uint64_t mask_32b=0xFFFFFFFFU, mask_14b=0x3FFFU, mask_4b=0xFU;
	// Reading the file.
	while (i < cargo->events_info.dim){
		for (j=0; i < cargo->events_info.dim && j < buff_size; j++, i++){
			// Timestamp.
			buff[j] = (uint64_t) arr[i].t & mask_32b; 
			// X address.
			buff[j] |= ((uint64_t) arr[i].x & mask_14b) << 32; 
			// Y address.
			buff[j] |= ((uint64_t) arr[i].y & mask_14b) << 46; 
			// Polarity.
			buff[j] |= ((uint64_t) arr[i].p & mask_4b) << 60; 
		}
		CHECK_FWRITE(fwrite(buff, sizeof(*buff), j, fp), j); 
	}
	fclose(fp); 
	free(buff); 
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

