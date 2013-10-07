#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"
#include <unistd.h>

int main(int argc, char** argv) {

	int err = 0;
	FILE* out = NULL;

    size_t size = 0;
	field_description* f_input = NULL;
	field_description* f_output = NULL;

	grib_handle *h = NULL;

	const void* buffer = NULL;

	/* int option_flags = 0; */
	/* void* b = 0; */

	if( argc < 2 )
    {
		printf("\n Usage: ./ecregrid_list_to_grib input output\n");
		return 1;
    }
	f_input =  ecregrid_get_field_description("list",&err);
	if(err != 0){
  		printf("ERROR: unable to create field \n");
		return 1;
	}

	err = ecregrid_set_file_type(f_input,"ascii");
	if(err){
		printf("ERROR: to set input \n");
		return 1;
	}

	err = ecregrid_set_missing_value(f_input,9999.0);
	if(err){
		printf("ERROR: to set input missing value \n");
		return 1;
	}

	f_output = ecregrid_get_field_description_general(&err);
	if(err != 0){
  		printf("ERROR: unable to create output field \n");
		return 1;
	}


	err = ecregrid_set_increments(f_output,1.0,1.0);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}

	err = ecregrid_set_edition_number(f_output,1);
	if(err != 0){
		printf("ERROR: to set output edition number \n");
		return 1;
	}
/*
	err = ecregrid_set_interpolation_method(f_output,"linear");
	if(err != 0){
		printf("ERROR: to set output interpolation method \n");
		return 1;
	}
*/

	h = ecregrid_process_read_from_file_to_grib_handle(f_input,argv[1],f_output,&err);
	if(err != 0){
		printf("ERROR: ecregrid_process_read_from_file_to_grib_handle \n");
		return 1;
	}

    out = fopen(argv[2],"w");
    if(!out) {
    	printf("ERROR: unable to open file %s\n",argv[2]);
        return 1;
    }

    /* Get the coded message in a buffer */
    GRIB_CHECK(grib_get_message(h,&buffer,&size),0);


	/* grib_dump_content(h,stdout,"serialize",option_flags,b); */


    /* Write the buffer in a file */
    if(fwrite(buffer,1,size,out) != size)
    {
        fclose(out);
        printf("ERROR: write error");
        return 1;
    }

	if(f_input)ecregrid_field_description_destroy(f_input);
	if(f_output)ecregrid_field_description_destroy(f_output);

	if(h)grib_handle_delete(h);

	fclose(out);


	return 0;
}
