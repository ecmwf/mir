#include <stdio.h>
#include <stdlib.h>

#include "grib_api.h"
#include "ecregrid_api.h"

int main(int argc, char** argv) {

	int err = 0;
	FILE* in = NULL;
	FILE* out = NULL;
	grib_handle *hin = NULL;
	grib_handle *hout = NULL;

	grib_handle *uin = NULL;
	grib_handle *vin = NULL;
	grib_handle *uout = NULL;
	grib_handle *vout = NULL;
	field_description* f = NULL;

	vector_grib_handle* vv = NULL;

	long paramID = 0;
	int uv = 0;

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
		printf("ERROR: unable to open input file %s\n",argv[1]);
		return 1;
	}
	out = fopen(argv[2],"w");
	if(!out) {
		printf("ERROR: unable to open output file file %s\n",argv[2]);
		return 1;
	}
	hout = grib_handle_new_from_template(NULL,"GRIB1");
	if (hout == NULL) {
		printf("ERROR: unable to create handle from template %s\n",argv[1]);
		return 1;
	}

	ecregrid_set_increments(f,1.0,1.0);
	ecregrid_set_south_pole(f,-30.0,10.0);
	ecregrid_set_area(f,80.0,-30.0,50.0,40.0);
	ecregrid_set_area(f,80.0,30.0,50.0,50.0);
	ecregrid_set_area(f,80.0,-30.0,50.0,-10.0);
	ecregrid_set_area(f,80.0,0,50.0,360.0);

	while ((hin = grib_handle_new_from_file(0,in,&err)) != NULL ) {
		GRIB_CHECK(err,0);
    	GRIB_CHECK(grib_get_long(hin,"paramId",&paramID),0);
		if( paramID == 138 || paramID == 131 || paramID == 165 ){
			printf("PARAMID %ld\n",paramID);
			uin = grib_handle_clone(hin);
			++uv;
			continue;
		}
		else if( paramID == 155 || paramID == 132 || paramID == 166 ){
			printf("PARAMID %ld\n",paramID);
			vin = grib_handle_clone(hin);
			++uv;
			continue;
		}
			printf("paramId %ld\n",paramID);
		hout = ecregrid_process_grib_handle_to_grib_handle(hin,f,&err);
		if(err){
			printf("ERROR: ecregrid_process_grib_handle_to_grib_handle \n");
			return 1;
		}
    	GRIB_CHECK(grib_get_message(hout,&buffer,&size),0);
    	if(fwrite(buffer,1,size,out) != size) {
   	   		  fclose(out);
        		printf("ERROR: write error");
        		return 1;
    	}
	}

	if( uv == 2) {
		printf("******************* %s\n",argv[1]);
		uout = grib_handle_new_from_template(NULL,"GRIB1");
		if (uout == NULL) {
			printf("ERROR: unable to create handle from template %s\n",argv[1]);
			return 1;
		}
		vout = grib_handle_new_from_template(NULL,"GRIB1");
		if (vout == NULL) {
			printf("ERROR: unable to create handle from template %s\n",argv[1]);
			return 1;
		}
		GRIB_CHECK(grib_get_long(uin,"paramId",&paramID),0);
			printf("U  PARAMID %ld\n",paramID);
		GRIB_CHECK(grib_get_long(vin,"paramId",&paramID),0);
			printf("V  PARAMID %ld\n",paramID);
		vv = ecregrid_process_vector_grib_handle_to_grib_handle(uin,vin,f,&err);

		if(err){
			printf("ERROR: ecregrid_process_wind_grib_handle_to_grib_handle \n");
			return 1;
		}

		uout = vv->one;
		vout = vv->two;

		GRIB_CHECK(grib_get_message(uout,&buffer,&size),0);
		if(fwrite(buffer,1,size,out) != size) {
			fclose(out);
        		printf("ERROR: write error");
        		return 1;
		}
		GRIB_CHECK(grib_get_message(vout,&buffer,&size),0);
		if(fwrite(buffer,1,size,out) != size) {
			fclose(out);
			printf("ERROR: write error");
			return 1;
		}
	}

	printf("ecregrid_process_wind_grib_handle_to_grib_handle --------- \n");

	ecregrid_field_description_destroy(f);
	grib_handle_delete(hin);
	grib_handle_delete(hout);

	grib_handle_delete(uin);
	grib_handle_delete(vin);

	grib_handle_delete(uout);
	grib_handle_delete(vout);

	fclose(in);
	fclose(out);
	return 0;
}
