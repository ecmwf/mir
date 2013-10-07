#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"

int main(int argc, char** argv) {

	int err = 0, i = 0;
	FILE* in = NULL;

	grib_handle *h = NULL;

	double lat[] = {54.3, 33.2, 48.1, 1.5};
	double lon[] = {22.1, 31.9, 77.3, 22.0};
	double value[4];
	long length = 4;

	if( argc < 1 )
    {
		printf("\n Usage: ./ecregrid_to_one_point inputgrib \n");
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


	err = ecregrid_list_of_points(h,lat,lon,value,&length);
	if(err != 0){
		printf("ERROR: ecregrid_one_point \n");
		return 1;
	}
	for( i = 0; i < length ; i++) {
		printf("lat: %f , lon: %f , value: %f \n", lat[i],lon[i],value[i]);
	}

	grib_handle_delete(h);

	fclose(in);
	return 0;
}
