/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

/*! \file ecregrid_api.h
  \brief Implementation of C interface
  This header file needs to be included by all C programs
*/
#ifndef ECREGRID_API_H
#define ECREGRID_API_H

/* #ifdef ECREGRID_GRIB */
#include "grib_api.h"
/* #endif */

/*! \mainpage Spectral Computation and Interpolation (ecRegrid)

ecRegrid is a recent software development at ECMWF to replace the INTERPOLATION routines within Emoslib and possibly 
other interpolation routines used in RD. This new development was necessary to give additional flexibility in 
answering current and future OD and RD requirements. 


\section modules Documentation

 - \ref api "Description of the C API"

 - \ref tool "Description of the ecregrid command line tool"

 - \ref mars "Description of how ecRegrid is used with MARS client"
*/


/*! \defgroup api API documentation

\section intro Overview

ecRegrid offers a comprehensive C interface for programmers to call the 
Tranformation and Interpolation routines which generate a new 
spectral or grid fields from spectral or grid on input. 
The fields can be GRIB formatted messages or arrays of numbers.

To use the C API you need to include one header file:

    #include "ecregrid_api.h"

All interface functions are decribed in detaill in ecregrid_api.h .

\example ecregrid_mars.c Example code how MARS could use the interpolation.

*/

/*! \defgroup mars how ecRegrid is used with MARS client 

\section intro Overview

   Mars client linked against ecRegrid library is installed on most of the main platforms. It could be used within the ECMWF typing:
   mars -i

*/

#ifdef __cplusplus
extern "C" {
#endif

/*! \brief   structure giving access to all possible features of field
 \details  field_description is used to descibe either input or required output field
*/
typedef struct FieldDescription field_description;

/*! \brief Create field description based on type of field provided.
 
 \param fieldType : field Type - GRIB API naming convention
 \param err  : 0 if OK, integer value on error 
 \return   the newly created field description
*/
field_description* ecregrid_get_field_description(const char* fieldType, int* err);

/*! \brief Create general field description
 
 \param err  : 0 if OK, integer value on error 
 \return   the newly created field description
*/
field_description* ecregrid_get_field_description_general(int* err);

/*! \brief Delete general field description
 
 \param f  : field description
*/
void ecregrid_field_description_destroy(field_description* f);


/* --------------- start grib_api ----------------------------- */
/* #ifdef ECREGRID_GRIB */

/*! \brief   structure giving access to grib handles of u,v component of wind
 \details  address  of this structure is used as return value in ecregrid api funtions which process vector data
*/
struct vector_grib_handle {
  grib_handle* one;
  grib_handle* two;
} ;
typedef struct vector_grib_handle      vector_grib_handle;
typedef struct FieldDescriptionGribApi field_description_grib_handle;

/*! \brief Create field description based on GRIB API handle.
 
 \param grib : GRIB handle
 \param err  : 0 if OK, integer value on error 
 \return   the newly created field description
 \sa ecregrid_get_field_description() ecregrid_get_field_description_general()
*/
field_description* ecregrid_get_field_description_from_grib_handle(grib_handle* grib, int* err);

/*! \brief Carries out the Tranformation between spectral or grid fields.
 
 \param inHandle : input GRIB API handle
 \param output   : field description for output field
 \param err      : 0 if OK, integer value on error 
 \return   the newly created GRIB API handle 
 \sa ecregrid_process_unpacked_to_grib_handle() ecregrid_process_vector_grib_handle_to_grib_handle() 
*/
grib_handle* ecregrid_process_grib_handle_to_grib_handle(grib_handle* inHandle, const field_description* output, int* err);

/*! \brief Carries out the Tranformation between spectral or grid fields.
 
 \param inHandle       : input GRIB API handle
 \param output         : field description for output field
 \param err            : 0 if OK, integer value on error 
 \param grib_set       : array of grib_values
 \param grib_set_count : number of array of grib_values
 \return   the newly created GRIB API handle 
 \sa ecregrid_process_unpacked_to_grib_handle() ecregrid_process_vector_grib_handle_to_grib_handle() 
*/
grib_handle* ecregrid_process_grib_handle_to_grib_handle_grib_set(grib_handle* inHandle, const field_description* output, const grib_values* grib_set, int grib_set_count, int* err);

/*! \brief Carries out the Tranformation between spectral or grid fields.
 
 \param input     : field description for input field
 \param inData    : input array of values
 \param inLength  : length of the input array of values
 \param output : field description for output field
 \param err    : 0 if OK, integer value on error 
 \return   the newly created GRIB API handle 
*/

grib_handle* ecregrid_process_unpacked_to_grib_handle(const field_description* input, double* inData, size_t inLength, const field_description* output, int* err);

/*! \brief Carries out the Tranformation between spectral or grid fields.
 
 \param inHandle  : input GRIB API handle
 \param output    : field description for output field
 \param outLength : length of the output array of values
 \param err       : 0 if OK, integer value on error 
 \return   the newly created array of values
*/

/*! \brief  Read from ascii and binary file and  Carries out the Tranformation between spectral or grid fields.
 
 \param input     : field description for input field
 \param fileName  : file name of input field
 \param output    : field description for output field
 \param err       : 0 if OK, integer value on error 
 \return   the newly created GRIB API handle 
*/
grib_handle* ecregrid_process_read_from_file_to_grib_handle(const field_description* input, const char* fileName, const field_description* output, int* err);

double* ecregrid_process_grib_handle_to_unpacked(grib_handle* inHandle, field_description* output, size_t* outLength, int* err);

/* -------------------------Wind---------------------------------- */
 
/*! \brief Carries out the Tranformation between spectral or grid fields for Wind parameters.
 
 \param inHandleU : input GRIB API handle for u component or vorticity
 \param inHandleV : input GRIB API handle for v component or divergency
 \param output    : field description for output fields
 \param err       : 0 if OK, integer value on error 
 \return   the newly created structure of two GRIB API handles for u and v
*/
vector_grib_handle* ecregrid_process_vector_grib_handle_to_grib_handle(grib_handle* inHandleU, grib_handle* inHandleV, const field_description* output, int* err);

/*! \brief Carries out the Tranformation between spectral or grid fields for Wind parameters.
 
 \param inHandleU      : input GRIB API handle for u component or vorticity
 \param inHandleV      : input GRIB API handle for v component or divergency
 \param output         : field description for output fields
 \param grib_set       : array of grib_values
 \param grib_set_count : number of array of grib_values
 \param err            : 0 if OK, integer value on error 
 \return   the newly created structure of two GRIB API handles for u and v
*/
vector_grib_handle* ecregrid_process_vector_grib_handle_to_grib_handle_grib_set(grib_handle* inHandleU, grib_handle* inHandleV, const field_description* output, const grib_values* grib_set, int grib_set_count, int* err);

/* -------------------------Wind---------------------------------- */
/* --------------- end grib_api --------------- */

/* #endif */


/*! \brief Carries out the Tranformation between spectral or grid fields.
 
 \param input     : field description for input field
 \param inData    : input array of values
 \param inLength  : length of the input array of values
 \param output    : field description for output field
 \param outLength : length of the output array of values
 \param err       : 0 if OK, integer value on error 
 \return   the newly created array of values
*/

double* ecregrid_process_unpacked_to_unpacked(const field_description* input, double* inData, size_t inLength, field_description* output, size_t* outLength, int* err);

/*! \brief Set Area 
 
 \param f     : field description 
 \param north 
 \param west
 \param south
 \param east
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_area(field_description* f, double north, double west, double south, double east);

/*! \brief Set South Pole of Rotation 
 
 \param f    : field description 
 \param lat  : latitude  of south pole of rotation
 \param lon  : longitude of south pole of rotation
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_south_pole(field_description* f, double lat, double lon);

/*! \brief Set Increments in Meridional and Paralel direction
 
 \param f    : field description 
 \param we   : increment in west-east direction
 \param ns   : increment in west-east direction
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_increments(field_description* f, double we, double ns);

/*! \brief Set Spectral Truncation
 
 \param f    : field description 
 \param t    : spectral truncation
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_truncation(field_description* f, int t);

/*! \brief Set Gaussian Number
 
 \param f    : field description 
 \param n    : gaussian number
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_gaussian_number(field_description* f, int n);

/*! \brief Set Scanning mode
 
 \param f    : field description 
 \param n    : scanning mode - ECREGRID convention
 \note ECREGRID convention for scanning mode (in brackets are specified GRIB API convention):
 \note 1 = north-south, west-east (iScansNegatively = 0, jScansPositively = 0) 
 \note 2 = south-north, west-east (iScansNegatively = 0, jScansPositively = 1)
 \note 3 = north-south, east-west (iScansNegatively = 1, jScansPositively = 0)
 \note 4 = south-north, east-west (iScansNegatively = 1, jScansPositively = 1)

 \return  0 if OK, integer value on error 
*/
int ecregrid_set_scanning_mode(field_description* f, int n);

/*! \brief Set Reduced grid(Gaussian or Lat-Lon)  definition
 
 \param f    : field description 
 \param size : the array of values - each value represent number of points along latitude
 \param size : length of the array of values
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_reduced_grid_definition(field_description* f, long* rgridDef, size_t size);

/*! \brief Set Reduced or Regular grid
 
 \param f    : field description 
 \param n    : Reduced or Regular
 \note 1 - reduced
 \note 0 - regular
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_is_reduced(field_description* f, int n);

/*! \brief Set Frame Number
	\details  Set Number of points across the Frame. Rest of the points to be removed from inside a frame boundary
 
 \param f    : field description 
 \param n    : Number of points across the Frame
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_frame_number(field_description* f, int n);


/*! \brief Set Bitmap file name
	\details  Set name of the file with bitmap definition. Definition to be used for removing points
 
 \param f    : field description 
 \param name : file name
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_bitmap_file(field_description* f, const char* name);

/*! \brief Set List of points file name
	\details  Set name of the file with lat-lon-value specification. For output grid could be just lat-lon specification.
 
 \param f    : field description 
 \param name : file name
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_list_of_points_file(field_description* f, const char* name);

/*! \brief Set file type
 
 \param f    : field description 
 \param type : file type
 \note File type could be: binary or ascii
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_file_type(field_description* f, const char* name);

/*! \brief Set List of points file type
 
 \param f    : field description 
 \param type : file type
 \note File type could be: binary or ascii
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_list_of_points_file_type(field_description* f, const char* type);

/*! \brief Set Legendre Polynomials Method
 
 \param f      : field description 
 \param method : Legendre Polynomials Method
 \note Following Legendre Polynomials methods available:
 \note on-fly  - Recalculate each time (default)
 \note file    - Read pre-calculated values from named file
 \note memmap  - Memory map
 \note shared  - Shared memory
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_legendre_polynomials_method(field_description* f, const char* method);

/*! \brief Set Interpolation method
 
 \param f      : field description 
 \param method : interpolation method
 \note Following interpolation methods available:
 \note bilinear  - Bilinear Interpolation(default)
 \note nearest   - Nearest Neighbour
 \note cubic     - Cubic interpolation with 12 points
 \note linear    - Linear interpolation
 \note linear-fit - Linear fit along the two lines of latitudes then linear fit along the line of meridian
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_interpolation_method(field_description* f, const char* method);

/*! \brief Set Lsm method
 
 \param f      : field description 
 \param method : lsm method
 \note Following lsm methods available:
 \note predefined - from Operational Model(default)
 \note gtopo 
 \note 10min USNAVY 10min from Emos lib 
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_lsm_method(field_description* f, const char* method);

/*! \brief Set Number of points along meridian and paralell
 
 \param f  : field description 
 \param ns : number of points in north-south direction
 \param we : number of points in west-east direction
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_number_of_points(field_description* f, long ns, long we);

/*! \brief Set if grid global west-east
 
 \param f      : field description 
 \param yesno  : set "yes" for global and "no" for nonglobal west-east grids
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_globalWestEast(field_description* f, const char* yesno);

/*! \brief Set Number of points along meridian and paralell
 
 \param f : field description 
 \param n : number of points used for interpolation
 \note this is valid only if nearest neighbour or average is choosen as interpolation method. Number of points could be 4,12,16(default for those mentioned),36 and 64 
 \return  0 if OK, integer value on error 
*/
int ecregrid_number_of_nearest_points(field_description* f, int n);

/*! \brief Set Level type of field
 
 \param f     : field description 
 \param type  : level type
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_level_type(field_description* f, const char* type);

/*! \brief Set Level
 
 \param f     : field description 
 \param level : level
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_level(field_description* f, int level);

/*! \brief Set Date
 
 \param f    : field description 
 \param date : date in format - YYYYMMDD
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_date(field_description* f, int date);

/*! \brief Set Time
 
 \param f    : field description 
 \param time : time in format - 1200
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_time(field_description* f, int time);

/*! \brief Set Units
 
 \param f    : field description 
 \param unit : unit
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_units(field_description* f, const char* unit);

/*! \brief Set Originating Centre
 
 \param f      : field description 
 \param centre : Originating Centre
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_centre(field_description* f, int centre);

/*! \brief Set Grib edition number
 
 \param f     : field description 
 \param grib  : Grib edition number
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_edition_number(field_description* f, int grib);

/*! \brief Set Step Units
 
 \param f    : field description 
 \param unit : Step Units
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_step_units(field_description* f, const char* unit);

/*! \brief Set Start Step
 
 \param f    : field description 
 \param step : Start Step
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_start_step(field_description* f, int step);

/*! \brief Set End Step
 
 \param f    : field description 
 \param step : End Step
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_end_step(field_description* f, int t);

/*! \brief Set Grid Type
 
 \param f    : field description 
 \param type : Grid Type
 \note grid type could be:
 \note reduced - for gaussian and lat-lon grids
 \note regular - for gaussian and lat-lon grids
 \note cell-centred - defines Regular lat-lon Cell Centered grid
 \note polar-stereographic - defines Polar Stereographic
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_grid_type(field_description* f, const char* type);

/*! \brief Set Extrapolation on Pole
    \details How to extrapolate on pole if interpolate from grid without point on pole 
 
 \param f    : field description 
 \param type : extrapolation type
 \note grid type could be:
 \note linear 
 \note average of last latitude
 \note nearest - Nearest Neighbour
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_extrapolate_on_pole(field_description* f, const char* type);

/*! \brief Set DerivedSubgridParameters type of processing data
 
 \param f    : field description 
 \param type : DerivedSubgridParameters type
 \note DerivedSubgridParameters type could be:
 \note anisotropy - Anisotropy
 \note orientation - Orientation
 \note slope - Slope
 \note stddev - StandardDeviation
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_derived_parameter(field_description* f, const char* param);

/*! \brief Set missing Value for Input data
 
 \param f     : field description 
 \param value : missing value
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_missing_value(field_description* f, double value);

/*! \brief Set parameter Id for Input data
 
 \param f     : field description 
 \param paramId : Parameter Id
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_parameter_id(field_description* f, int paramId);

/*! \brief Set table for Input data
 
 \param f     : field description 
 \param table : local table number
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_table(field_description* f, int table);

/*! \brief Set auresol parameter for Input data
 
 \param f     : field description 
 \param m : identifier for auresol parameter
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_auresol(field_description* f, const char* m);

/* --------------------------------------------------------------------------- */

/*! \brief Get North boundary of interpolated field
 
 \param f     : field description 
 \param north : the address of double where the data will be retreived
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_area_north(const field_description* f, double* north);

/*! \brief Get West boundary of interpolated field
 
 \param f     : field description 
 \param west  : the address of double where the data will be retreived
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_area_west(const field_description* f, double* west);

/*! \brief Get South boundary of interpolated field
 
 \param f     : field description 
 \param south : the address of double where the data will be retreived
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_area_south(const field_description* f, double* south);

/*! \brief Get East boundary of interpolated field
 
 \param f     : field description 
 \param east  : the address of double where the data will be retreived
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_area_east(const field_description* f, double* east);

/*! \brief Get Latitude of South Pole of Rotation of interpolated field
 
 \param f     : field description 
 \param lat   : the address of double where the data will be retreived
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_south_pole_lat(const field_description* f, double* lat);

/*! \brief Get Longitude of South Pole of Rotation of interpolated field
 
 \param f     : field description 
 \param lon   : the address of double where the data will be retreived
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_south_pole_lon(const field_description* f, double* lon);

/*! \brief Get Parameter Number of interpolated field
 
 \param f      : field description 
 \param number : parameter number
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_parameter_number(const field_description* f, int* number);

/*! \brief Get Local Table Number of interpolated field
 
 \param f     : field description 
 \param table : painter for parameter table
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_parameter_table(const field_description* f, int* table);

/*! \brief Get Date of interpolated field
 
 \param f    : field description 
 \param date : pointer returning the date
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_date(const field_description* f, int* date);

/*! \brief Get Time of interpolated field
 
 \param f    : field description 
 \param time : pointer returning the time
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_time(const field_description* f, int* time);

/*! \brief Get Scanning Mode of interpolated field
 
 \param f     : field description 
 \param scan  : scan mode identifier
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_scanning_mode(const field_description* f, int* scan);

/*! \brief Get Gaussian Number of interpolated field
 
 \param f      : field description 
 \param number : pointer returning the Gaussian number
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_gaussian_number(const field_description* f, int* number);

/*! \brief Get Truncation of interpolated field
 
 \param f     : field description 
 \param trunc : pointer returning truncation
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_truncation(const field_description* f, int* trunc);

/*! \brief Get west-east increment
 
 \param f   : field description 
 \param we  : pointer returning the increment in west and east direction
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_west_east_increment(const field_description* f, double* we);

/*! \brief Get north-south increment
 
 \param f  : field description 
 \param ns : pointer returning the increment in north and south direction
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_north_south_increment(const field_description* f, double* ns);

/*! \brief Get north-south number of points
 
 \param f  : field description 
 \param ns : pointer returning the number of points in north and south direction
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_north_south_number_of_points(const field_description* f, long* ns);

/*! \brief Get west-east number of points
 
 \param f  : field description 
 \param we : pointer returning the number of points in west and east direction
 \return  0 if OK, integer value on error 
*/
int ecregrid_get_west_east_number_of_points(const field_description* f, long* we);

/*! \brief Get version number of ECREGRID
 
 \return in format ECREGRID_MAJOR_VERSION*10000+ECREGRID_MINOR_VERSION*100+ECREGRID_REVISION_VERSION
*/
long ecregrid_get_version(void);

int ecregrid_one_point(grib_handle* inHandle, double* lat, double* lon , double* value);

int ecregrid_list_of_points(grib_handle* inHandle, double* lat, double* lon , double* values, long* length);

/*! \brief Set Bits Per Value
	\details  Sets the number of bits per stored value in the output grib file. Corresponds to MARS ACCURACY keyword.
 
 \param f    : field description 
 \param n    : Number of bits per value
 \return  0 if OK, integer value on error 
*/
int ecregrid_set_bits_per_value(field_description* f, int n);
#ifdef __cplusplus
}
#endif

#endif
