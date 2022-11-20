#include "events.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

event_array_t void_event_array(){
	return (event_array_t) {NULL, NULL, NULL, NULL, 0, 0}; 
}

event_t void_event(){
	return (event_t) {0, 0, 0, 0}; 
}

unsigned int is_void_event_array(event_array_t* arr){
	return (arr->t_arr == NULL && arr->x_arr == NULL && arr->y_arr == NULL && arr->p_arr == NULL && arr->allocated_space==0 && arr->dim==0);
}

void check_event_array_allocation(event_array_t* arr){
	if (arr->t_arr == NULL || arr->x_arr == NULL || arr->y_arr == NULL || arr->p_arr == NULL){
		free_event_array(arr);
		*arr = void_event_array(); 
	}
	return;
}

		
DLLEXPORT void free_event_array(event_array_t* arr){
	free(arr->t_arr); 
	free(arr->x_arr); 
	free(arr->y_arr); 
	free(arr->p_arr); 
	arr->allocated_space=0; 
	arr->dim=0; 
}

event_array_t realloc_event_array(event_array_t* arr, size_t new_dim){
	// Timestamp.
	arr->t_arr = (timestamp_t*) realloc(arr->t_arr, new_dim * sizeof(timestamp_t));
	// X coordinate.
	arr->x_arr = (address_t*) realloc(arr->x_arr, new_dim * sizeof(address_t));
	// Y coordinate.
	arr->y_arr = (address_t*) realloc(arr->y_arr, new_dim * sizeof(address_t));
	// Polarity.
	arr->p_arr = (polarity_t*) realloc(arr->p_arr, new_dim * sizeof(polarity_t));
	// Checking reallocation.
	check_event_array_allocation(arr); 
	if (!is_void_event_array(arr))
		arr->allocated_space = new_dim; 
	return *arr;
}

event_array_t malloc_event_array(size_t dim){
	event_array_t arr; 
	arr.dim = 0; arr.allocated_space = dim; 
	// Allocating the array of events.
	// Timestamp.
	arr.t_arr = (timestamp_t*) malloc(arr.allocated_space * sizeof(timestamp_t));
	// X coordinate.
	arr.x_arr = (address_t*) malloc(arr.allocated_space * sizeof(address_t));
	// Y coordinate.
	arr.y_arr = (address_t*) malloc(arr.allocated_space * sizeof(address_t));
	// Polarity.
	arr.p_arr = (polarity_t*) malloc(arr.allocated_space * sizeof(polarity_t));
	// Error checking.
	check_event_array_allocation(&arr); 
	if (!is_void_event_array(&arr)){
		arr.allocated_space = dim; 
		arr.dim = 0; 
	}
	return arr; 
}
