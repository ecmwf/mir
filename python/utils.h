#include <ecregrid_api.h>

#ifdef __cplusplus
extern "C" {
#endif

grib_handle* _ecregrid_util_local_grib_handle_from_message ( char* data, size_t buflen );
int _ecregrid_util_message_from_local_grib_handle(grib_handle* h, const void **msg, size_t *size) ;

grib_handle* _ecregrid_util_vector_grib_handle_get_at(vector_grib_handle* vec, size_t index);

int _get_handle_from_field_description(field_description* fd);
int _add_field_description(field_description* fd);
field_description* _get_field_description_from_handle(int handle);
field_description* _remove_field_description(int handle);

#ifdef __cplusplus
}
#endif
