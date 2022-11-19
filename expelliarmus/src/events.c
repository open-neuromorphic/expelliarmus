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

void add_event(const event_t* event, event_array_t* arr, size_t i){
	if (i >= arr->allocated_space){
		event_array_t tmp = realloc_event_array(arr, 2*arr->allocated_space); // Doubling the storage.
		if (is_void_event_array(&tmp))
			free_event_array(arr); 
		*arr = tmp; 
	}
	// Appending the event to the array.
	if (!is_void_event_array(arr)){
		arr->t_arr[i] = event->t; 
		arr->x_arr[i] = event->x; 
		arr->y_arr[i] = event->y; 
		arr->p_arr[i] = event->p; 
		arr->dim++; 
	} 
	return; 
}

event_array_t realloc_event_array(event_array_t* arr, size_t new_dim){
	// Timestamp.
	arr->t_arr = (timestamp_t*) realloc(arr->t_arr, new_dim * sizeof(timestamp_t));
	// X coordinate.
	arr->x_arr = (pixel_t*) realloc(arr->x_arr, new_dim * sizeof(pixel_t));
	// Y coordinate.
	arr->y_arr = (pixel_t*) realloc(arr->y_arr, new_dim * sizeof(pixel_t));
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
	arr.x_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
	// Y coordinate.
	arr.y_arr = (pixel_t*) malloc(arr.allocated_space * sizeof(pixel_t));
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
