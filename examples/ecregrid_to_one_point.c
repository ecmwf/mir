#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"

int main(int argc, char** argv) {

	int err = 0;
	FILE* in = NULL;

	grib_handle *h = NULL;

	/* double lat = 54.3; */
	double lat = 89.753;

	double lon = 22.1;
	double value = 0;

	if( argc < 2 )
    {
		printf("Usage: ./ecregrid_to_one_point inputgrib \n");
		return 1;
    }

	in = fopen(argv[1],"r");
    if(!in)
    {
		perror(argv[1]);
        return 1;
    }

	/* create new handle from a message in a file*/
	h = grib_handle_new_from_file(0,in,&err);
	if (h == NULL) {
		fprintf(stderr,"unable to create handle from file : %s\n", argv[1]);
		return err;
	}


	err = ecregrid_one_point(h,&lat,&lon,&value);
	if(err != 0){
		fprintf(stderr,"ecregrid_one_point %d \n",err);
		return err;
	}
	printf("lat: %f , lon: %f , value: %f \n", lat,lon,value);

	grib_handle_delete(h);

	fclose(in);
	return 0;
}
