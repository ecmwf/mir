
/* wraps the raw C interface */

%module ecregrid_swig
%include "typemaps.i"
%include "cpointer.i"
%include "cstring.i"
%include "cdata.i"
%include "carrays.i"


%{
#define SWIG_FILE_WITH_INIT
#include "../src/ecregrid_api.h"
#include "../src/ecregrid_config.h"
#include "utils.h"
%}


/* Utility functions for grib handling. From utils.c */
grib_handle* _ecregrid_util_vector_grib_handle_get_at(vector_grib_handle* INPUT, size_t index);
grib_handle* _ecregrid_util_local_grib_handle_from_message ( char* msg, size_t len );

%cstring_output_allocate_size(const void **binmsg, size_t *binmsglen,);
int _ecregrid_util_message_from_local_grib_handle(grib_handle* INPUT, const void **binmsg, size_t *binmsglen);
%clear const void **binmsg, size_t *binmsglen;

int _get_handle_from_field_description(field_description* INPUT);
int _add_field_description(field_description* INPUT);
field_description* _get_field_description_from_handle(int handle);
field_description* _remove_field_description(int handle);

/* Wrappers for functions defined in ecregrid_api.h */

long ecregrid_get_version();

field_description* ecregrid_get_field_description(const char* fieldType, int* OUTPUT);
field_description* ecregrid_get_field_description_general(int* OUTPUT);
field_description* ecregrid_get_field_description_from_grib_handle(grib_handle* INPUT, int* OUTPUT);
void ecregrid_field_description_destroy(field_description* INPUT);

int ecregrid_set_area(field_description* INPUT, double north, double west, double south, double east);
int ecregrid_set_south_pole(field_description* INPUT, double lat, double lon);
int ecregrid_set_increments(field_description* INPUT, double we, double ns);
int ecregrid_set_truncation(field_description* INPUT, int t);
int ecregrid_set_gaussian_number(field_description* INPUT, int n);
int ecregrid_set_scanning_mode(field_description* INPUT, int n);

int ecregrid_one_point(grib_handle* INPUT, double* INPUT, double* INPUT , double* OUTPUT);

grib_handle* ecregrid_process_grib_handle_to_grib_handle(grib_handle* INPUT, const field_description* INPUT, int* OUTPUT);


int ecregrid_set_reduced_grid_definition(field_description* INPUT, long* INPUT, size_t size);
int ecregrid_set_is_reduced(field_description* INPUT, int n);
int ecregrid_set_frame_number(field_description* INPUT, int n);
int ecregrid_set_bitmap_file(field_description* INPUT, const char* INPUT);
int ecregrid_set_list_of_points_file(field_description* INPUT, const char* INPUT);
int ecregrid_set_file_type(field_description* INPUT, const char* INPUT);
int ecregrid_set_list_of_points_file_type(field_description* INPUT, const char* INPUT);
int ecregrid_set_interpolation_method(field_description* INPUT, const char* INPUT);


int ecregrid_set_lsm_method(field_description* INPUT, const char* INPUT);
int ecregrid_set_number_of_points(field_description* INPUT, long ns, long we);
int ecregrid_number_of_nearest_points(field_description* INPUT, int n);
int ecregrid_set_date(field_description* INPUT, int date);
int ecregrid_set_time(field_description* INPUT, int time);

int ecregrid_set_edition_number(field_description* INPUT, int grib);
int ecregrid_set_start_step(field_description* INPUT, int step);
int ecregrid_set_end_step(field_description* INPUT, int t);
int ecregrid_set_grid_type(field_description* INPUT, const char* INPUT);
int ecregrid_set_extrapolate_on_pole(field_description* INPUT, const char* INPUT);
int ecregrid_set_derived_parameter(field_description* INPUT, const char* INPUT);
int ecregrid_set_missing_value(field_description* INPUT, double value);
int ecregrid_set_parameter_id(field_description* INPUT, int paramId);
int ecregrid_set_table(field_description* INPUT, int table);
int ecregrid_get_area_north(const field_description* INPUT, double* INPUT);
int ecregrid_get_area_west(const field_description* INPUT, double* INPUT);
int ecregrid_get_area_south(const field_description* INPUT, double* INPUT);
int ecregrid_get_area_east(const field_description* INPUT, double* INPUT);


%array_class(double, doublearray);

double* ecregrid_process_grib_handle_to_unpacked(grib_handle* INPUT, field_description* INPUT, size_t* OUTPUT, int* OUTPUT);
grib_handle* ecregrid_process_unpacked_to_grib_handle(const field_description* INPUT, double* inData, size_t inLength, const field_description* INPUT, int* OUTPUT);
double* ecregrid_process_unpacked_to_unpacked(const field_description* INPUT, double* inData, size_t inLength, field_description* INPUT, size_t* OUTPUT, int* OUTPUT);
grib_handle* ecregrid_process_read_from_file_to_grib_handle(const field_description* INPUT, const char* INPUT, const field_description* INPUT, int* OUTPUT);

vector_grib_handle* ecregrid_process_vector_grib_handle_to_grib_handle(grib_handle* INPUT, grib_handle* INPUT, const field_description* INPUT, int* OUTPUT);

int ecregrid_set_auresol(field_description* INPUT, const char* INPUT); 

