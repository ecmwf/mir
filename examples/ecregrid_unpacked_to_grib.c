#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"
#include <unistd.h>

int main(int argc, char** argv) {

	int err = 0;
	FILE* in = NULL;
	FILE* out = NULL;

    size_t size = 0;
	field_description* f_input;
	field_description* f_output;
	double* inData = 0;
	size_t inLength = 0;

	grib_handle *h = NULL;

	const void* buffer = NULL;


	if( argc < 2 )
    {
		printf("\n Usage: ./ecregrid_unpacked_to_grib input output\n");
		return 1;
    }
/*
	field_description* f = ecregrid_field_create("regular_ll",&err);
	if(err != 0){
  		printf("ERROR: unable to create field \n");
		return 1;
	}

*/
	f_input = ecregrid_get_field_description_general(&err);
	if(err != 0){
  		printf("ERROR: unable to create input field \n");
		return 1;
	}

	f_output = ecregrid_get_field_description_general(&err);
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
	fseek(in, 0, SEEK_END);
	size = ftell(in);
	rewind(in);

	inData = (double*)malloc(size);

	inLength = size / sizeof(double);


	if(fread(inData, sizeof(double),inLength,in) != inLength) {
        fclose(in);
        printf("ERROR: read error");
        return 1;
    }

/*
	err = ecregrid_set_increments(f_input,0.2,0.2);
	err = ecregrid_set_south_pole(f_input,0,80.0);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}
*/
	err = ecregrid_set_gaussian_number(f_input,640);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}
	err = ecregrid_set_is_reduced(f_input,1);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}

	h = ecregrid_process_unpacked_to_grib_handle(f_input,inData,inLength,f_output,&err);
	if(err != 0){
		printf("ERROR: ecregrid_process_unpacked \n");
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

    out = fopen(argv[2],"w");
    if(!out) {
    	printf("ERROR: unable to open file %s\n",argv[2]);
        return 1;
    }

    /* Write the buffer in a file */
    if(fwrite(buffer,1,size,out) != size)
    {
        fclose(out);
        printf("ERROR: write error");
        return 1;
    }

	ecregrid_field_description_destroy(f_input);
	ecregrid_field_description_destroy(f_output);
	grib_handle_delete(h);

	fclose(in);
	fclose(out);
	return 0;
}
