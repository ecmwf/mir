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

extern "C" fortint intout_(const char *name,
                           const fortint ints[],
                           const fortfloat reals[],
                           const char *value,
                           const fortint name_len,
                           const fortint value_len);

extern "C" fortint intuvp2_(const void *vort_grib_in,
                            const void *div_grib_in,
                            const fortint &length_in,
                            void *u_grib_out,
                            void *v_grib_out,
                            fortint &length_out);
