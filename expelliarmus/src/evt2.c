#include "evt2.h"
#include <stdio.h> 

DLLEXPORT size_t measure_evt2(const char* fpath, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	CHECK_JUMP_HEADER(jump_header(fp, NULL, 0U)); 	

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

	if (cargo->events_info.bytes_done == 0){
		CHECK_JUMP_HEADER((cargo->events_info.bytes_done=jump_header(fp, NULL, 0U))); 	
		cargo->time_high = 0; 
		cargo->last_t = 0; 
	} else
		CHECK_FSEEK(fseek(fp, (long)cargo->events_info.bytes_done, SEEK_SET)); 

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
	while (i < cargo->events_info.dim && (values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
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
		cargo->events_info.bytes_done += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	cargo->events_info.dim = i;
	if (i==0)
		return -1; 
	return 0; 
}

DLLEXPORT int save_evt2(const char* fpath, event_t* arr, evt2_cargo_t* cargo, size_t buff_size){
	char header[150]; 
	sprintf(header, "%c This EVT2 file has been generated through expelliarmus (https://github.com/fabhertz95/expelliarmus.git) %c%c evt 2.0 %c", (char)HEADER_START, (char)HEADER_END, (char)HEADER_START, (char)HEADER_END); 
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

	// Buffer to read the file.
	uint32_t* buff = (uint32_t*) malloc(buff_size * sizeof(uint32_t)); 
	CHECK_BUFF_ALLOCATION(buff); 

	// Indices to access the input file.
	size_t i=0, j=0; 
	// Masks to extract bits.
	const uint32_t mask_6b=0x3FU, mask_11b=0x7FFU, mask_28b=0xFFFFFFFU;
	// Values to handle overflows.
	uint32_t time_high=0; 
	// Reading the file.
	while (i < cargo->events_info.dim){
		for (j=0; i < cargo->events_info.dim && j < buff_size; j++){
			buff[j] = 0; 
			// Extracting 28 MSBs of the time stamp.
			time_high = (((uint32_t)arr[i].t>>6) & mask_28b); 
			// If it is different from before, we add a EVT2_TIME_HIGH to the stream.
			if (cargo->time_high != time_high || cargo->events_info.bytes_done == header_len){
				buff[j] |= ((uint32_t)EVT3_TIME_HIGH) << 28; 
				buff[j] |= time_high; 
				cargo->time_high = time_high; 
			} else {
				// Event type. 
				buff[j] |= ((uint32_t)(arr[i].p ? EVT2_CD_ON : EVT2_CD_OFF)) << 28; 
				// Time low.
				buff[j] |= (((uint32_t) arr[i].t) & mask_6b) << 22; 
				// X address.
				buff[j] |= (((uint32_t) arr[i].x) & mask_11b) << 11; 
				// Y address.
				buff[j] |= ((uint32_t)arr[i++].y) & mask_11b; 
			}
		}
		CHECK_FWRITE(fwrite(buff, sizeof(*buff), j, fp), j); 
		cargo->events_info.bytes_done += j*sizeof(*buff); 
	}
	fclose(fp); 
	free(buff); 
	return 0; 
}

DLLEXPORT size_t cut_evt2(const char* fpath_in, const char* fpath_out, size_t new_duration, size_t buff_size){
	FILE* fp_in = fopen(fpath_in, "rb"); 
	CUT_CHECK_FILE(fp_in, fpath_in); 
	FILE* fp_out = fopen(fpath_out, "wb"); 
	CUT_CHECK_FILE(fp_out, fpath_out); 

	// Jumping over the headers.
	CUT_CHECK_JUMP_HEADER(jump_header(fp_in, fp_out, 1U)); 	

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

