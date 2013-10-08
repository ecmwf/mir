#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"

int main(int argc, char** argv) {

	int err = 0;
	FILE* in = NULL;
	FILE* out = NULL;
	grib_handle *h = NULL;
	grib_handle *hh = NULL;
	field_description* f = NULL;

	const void* buffer = NULL;
    size_t size = 0;

	if( argc < 2 )
    {
		printf("\n Usage: ./ecregrid_mars inputgrib outputgrib\n");
		return 1;
    }

	f = ecregrid_get_field_description("regular_ll",&err);
	if(err != 0){
  		printf("ERROR: unable to create field \n");
		return 1;
	}

	in = fopen(argv[1],"r");
    if(!in)
    {
    	printf("ERROR: unable to open file %s\n",argv[1]);
        return 1;
    }

	/* create new handle from a message in a file*/
	h = grib_handle_new_from_file(0,in,&err);
	if (h == NULL) {
		printf("ERROR: unable to create handle from file %s\n",argv[1]);
		return 1;
	}

	err = ecregrid_set_increments(f,0.25,0.25);

	hh = ecregrid_process_grib_handle_to_grib_handle(h,f,&err);
	if(err != 0){
		printf("ERROR: ecregrid_process_grib_handle \n");
		return 1;
	}

    /* Get the coded message in a buffer */
    GRIB_CHECK(grib_get_message(hh,&buffer,&size),0);

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


	ecregrid_field_description_destroy(f);
	grib_handle_delete(h);
	grib_handle_delete(hh);

	fclose(in);
	fclose(out);
	return 0;
}
