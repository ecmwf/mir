#include <stdio.h>
#include <stdlib.h>

#include "ecregrid_api.h"

int main(int argc, char** argv) {

	int err = 0;
	FILE* in = NULL;
	FILE* out = NULL;

    size_t size = 0;
	field_description* f_input;
	field_description* f_output;
	double* inData = 0;
	size_t   inLength = 0; 
	size_t  outLength = 0;
	double* outData = 0;


	if( argc < 2 )
    {
		printf("\n Usage: ./ecregrid_dissemination input output\n");
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

	err = ecregrid_set_gaussian_number(f_input,200);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}

	err = ecregrid_set_increments(f_output,1.0,1.0);
	if(err != 0){
		printf("ERROR: unable to create field \n");
		return 1;
	}

	outData = ecregrid_process_unpacked_to_unpacked(f_input,inData,inLength,f_output,&outLength, &err);
	if(err != 0){
		printf("ERROR: ecregrid_process_unpacked \n");
		return 1;
	}

    out = fopen(argv[2],"w");
    if(!out) {
    	printf("ERROR: unable to open file %s\n",argv[2]);
        return 1;
    }

    /* Write data in a file */
    if(fwrite(outData,sizeof(double),outLength,out) != outLength)
    {
        fclose(out);
        printf("ERROR: write error");
        return 1;
    }

	ecregrid_field_description_destroy(f_input);
	ecregrid_field_description_destroy(f_output);
	free(outData);

	fclose(in);
	fclose(out);
	return 0;
}
