#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"

int main(int argc, char** argv) {

	int err = 0;
	FILE* in = NULL;
	FILE* out = NULL;
	const void* buffer = NULL;
    size_t size = 0;
	field_description* f_output_1;
	field_description* f_output_2;
	grib_handle *handle_in = NULL;
	grib_handle *handle_out = NULL;
	size_t outLength = 0;
	double* outData = 0;

	if( argc < 2 )
    {
		printf("\n Usage: ./ecregrid_dissemination_mix input output\n");
		return 1;
    }

	f_output_1 = ecregrid_get_field_description("regular_ll",&err);
	if(err != 0){
  		printf("ERROR: unable to create field \n");
		return 1;
	}

	f_output_2 = ecregrid_get_field_description_general(&err);
	if(err != 0){
  		printf("ERROR: unable to create output field \n");
		return 1;
	}

	in = fopen(argv[1],"r");
    if(!in)
    {
    	printf("ERROR: unable to open file %s\n",argv[1]);
        return 1;
    }

	/* create new handle from a message in a file*/
    handle_in = grib_handle_new_from_file(0,in,&err);
    if (handle_in == NULL) {
        printf("ERROR: unable to create handle from file %s\n",argv[1]);
        return 1;
    }

	handle_out = grib_handle_new_from_template(NULL,"GRIB1");
	if (handle_out == NULL) {
		printf("ERROR: unable to create handle from template %s\n",argv[1]);
		return 1;
	}

	err = ecregrid_set_increments(f_output_1,1.0,1.0);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}

	outData = ecregrid_process_grib_handle_to_unpacked(handle_in,f_output_1,&outLength,&err);
	if(err != 0){
		printf("ERROR: ecregrid_process_grib_handle_to_unpacked \n");
		return 1;
	}

	printf("==========================================================\n");

	err = ecregrid_set_area(f_output_2,80.0,-30.0,50.0,40.0);
	if(err != 0){
		printf("ERROR: unable to set field description \n");
		return 1;
	}

	handle_out = ecregrid_process_unpacked_to_grib_handle(f_output_1,outData,outLength,f_output_2,&err);
	if(err != 0){
		printf("ERROR: ecregrid_process_unpacked_to_grib_handle \n");
		return 1;
	}

    out = fopen(argv[2],"w");
    if(!out) {
    	printf("ERROR: unable to open file %s\n",argv[2]);
        return 1;
    }

    /* Get the coded message in a buffer */
    GRIB_CHECK(grib_get_message(handle_out,&buffer,&size),0);

    /* Write the buffer in a file */
    if(fwrite(buffer,1,size,out) != size)
    {
        fclose(out);
        printf("ERROR: write error");
        return 1;
    }


	ecregrid_field_description_destroy(f_output_1);
	ecregrid_field_description_destroy(f_output_2);

	grib_handle_delete(handle_in);
	/* grib_handle_delete(handle_out); */

	fclose(in);
	fclose(out);
	return 0;
}
