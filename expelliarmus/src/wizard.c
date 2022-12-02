#include "wizard.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

size_t jump_header(FILE* fp_in, FILE* fp_out, uint8_t copy_file){
	size_t bytes_done = 0; 
	uint8_t c, header_begins; 
	 do {
		header_begins = 1; 
	   	do { 
			bytes_done += fread(&c, 1, 1, fp_in); 
			if (header_begins && c != HEADER_START){
				CHECK_FSEEK(fseek(fp_in, -1, SEEK_CUR)); 
				return --bytes_done; 
			} else
				header_begins = 0; 
			if (copy_file)
				CHECK_FWRITE(fwrite(&c, 1, 1, fp_out), 1);  
		} while (c != HEADER_END); 
	} while (1); 
	return 0; 
}
