#include "potter.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <Python.h>

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

// PyMODINIT_FUNC PyInit_potter(void){
// }

void free_event_array(event_array_t arr){
	free(arr); 
	return; 
}

void append_event(const struct event_s* event, event_array_t* arr, size_t* allocated_space, size_t* i){
	event_array_t arr_tmp = *arr; 
	if (*i >= *allocated_space){
	  	// Doubling the storage. 
	   	event_array_t tmp = (event_array_t) realloc(arr_tmp, 2*(*allocated_space)*sizeof(event_t)); 
		CHECK_ALLOCATION(tmp); 
		arr_tmp = tmp; 
		*allocated_space *= 2; 
	}
	arr_tmp[*i+T_POS] = event->t;
	arr_tmp[*i+X_POS] = event->x;
	arr_tmp[*i+Y_POS] = event->y;
	arr_tmp[*i+P_POS] = event->p;
	*i += 4;
	*arr = arr_tmp; 
	return; 
}

event_array_t read_dat(const char* fpath, size_t* dim, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp); 
#ifdef VERBOSE
			printf("%c", pt); 
#endif
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp); 
		if (pt != 0x25) break; 
#ifdef VERBOSE
		printf("%c", pt); 
#endif
	} while (1); 

#ifdef VERBOSE
	printf("Binary event type: 0x%x.\n", pt); 
#endif
	fread(&pt, 1, 1, fp); 
#ifdef VERBOSE
	printf("Binary event size: 0x%x.\n", pt); 
#endif

	// Now we can start to have some fun.
	event_array_t arr; 
	size_t allocated_space=DEFAULT_ARRAY_DIM; 
	// Allocating the array of events.
  	arr= (event_array_t) malloc(allocated_space * sizeof(event_t)); 	
	CHECK_ALLOCATION(arr); 
	
	// Buffer to read binary data.
	uint32_t* buff = (uint32_t*) malloc(2*buff_size * sizeof(uint32_t));
	CHECK_ALLOCATION(buff); 
	
	// Temporary event data structure.
	struct event_s event_tmp; event_tmp.x=0; event_tmp.y=0; event_tmp.t=0; event_tmp.p=0;  
	size_t values_read=0, j=0, i=0; 
	const uint32_t mask_4b=0xFU, mask_14b=0x3FFFU;
	event_t time_ovfs=0, time_val=0; 
	while ((values_read = fread(buff, sizeof(*buff), 2*buff_size, fp)) > 0){
		for (j=0; j<values_read; j+=2){
			// Event timestamp.
			if (((event_t)buff[j]) < time_val) // Overflow.
				time_ovfs++; 
			time_val = (event_t) buff[j]; 
			event_tmp.t = (time_ovfs<<32) | time_val; 
			// Event polarity.
			event_tmp.p = (event_t) ((buff[j+1] >> 28) & mask_4b); 
			// Event y address.
			event_tmp.y = (event_t) ((buff[j+1] >> 14) & mask_14b); 
			// Event x address. 
			event_tmp.x = (event_t) (buff[j+1] & mask_14b); 
			append_event(&event_tmp, &arr, &allocated_space, &i); 
		}
	}
	free(buff); 
	fclose(fp); 
	// Reallocating to save memory.
	event_array_t tmp = (event_array_t) realloc(arr, i*sizeof(event_t)); 
	CHECK_ALLOCATION(tmp); 
	arr= tmp; 
	*dim = i; 
	return arr; 
}	
event_array_t read_evt2(const char* fpath, size_t* dim, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp); 
#ifdef VERBOSE
			printf("%c", pt); 
#endif
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp); 
		if (pt != 0x25) break; 
#ifdef VERBOSE
		printf("%c", pt); 
#endif
	} while (1); 
	
	// Coming back to previous byte.
	fseek(fp, -1, SEEK_CUR); 

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
	event_t time_high=0, time_low=0; 
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
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
	}
	fclose(fp); 
	free(buff); 
	// Reallocating to save memory.
	event_array_t tmp = realloc(arr, i*sizeof(event_t)); 
	CHECK_ALLOCATION(tmp); 
	arr= tmp; 
	*dim = i; 
	return arr; 
}

event_array_t read_evt3(const char* fpath, size_t* dim, size_t buff_size){
	FILE* fp = fopen(fpath, "rb"); 
	CHECK_FILE(fp, fpath); 

	// Jumping over the headers.
	uint8_t pt; 
	 do {
	   	do { 
			fread(&pt, 1, 1, fp); 
#ifdef VERBOSE
			printf("%c", pt); 
#endif
		} while (pt != 0x0A); 
		fread(&pt, 1, 1, fp); 
		if (pt != 0x25) break; 
#ifdef VERBOSE
		printf("%c", pt); 
#endif
	} while (1); 
	
	// Coming back to previous byte.
	fseek(fp, -1, SEEK_CUR); 

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
	while ((values_read = fread(buff, sizeof(*buff), buff_size, fp)) > 0){
		for (j=0; j<values_read; j++){
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
	return arr; 
}
