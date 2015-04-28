/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

typedef int fortint;
typedef double fortfloat;
typedef void (*emos_cb_proc)(char *);

extern "C" {
    fortint intout_(char *name, fortint *ints, fortfloat *reals, const char *value, fortint, fortint) ;
    fortint intin_(char *name, fortint *ints, fortfloat *reals, const char *value, fortint, fortint) ;
    fortint intf_(char *, fortint *, fortfloat *, char *, fortint *, fortfloat *) ;
    fortint intf2(char *grib_in, fortint *length_in, char *grib_out, fortint *length_out) ;
    fortint intuvs2_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) ;
    fortint intuvp2_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out);
    fortint intvect2_(char *u_grib_in, char *v_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out);
    fortint intuvs_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) ;
    fortint intuvp_(char *vort_grib_in, char *div_grib_in, fortint *length_in, char *vort_grib_out, char *div_grib_out, fortint *length_out) ;
    fortint intvect_(char *u_grib_in, char *v_grib_in, fortint *length_in, char *u_grib_out, char *v_grib_out, fortint *length_out) ;
    fortint iscrsz_() ;
    fortint ibasini_(fortint *force) ;
    void intlogm_(fortint (*)(char *, fortint));
    void intlogs(emos_cb_proc proc);
    fortint areachk_(fortfloat *we, fortfloat *ns,
                     fortfloat *north, fortfloat *west,
                     fortfloat *south, fortfloat *east) ;
    fortint emosnum_(fortint *value) ;

}
