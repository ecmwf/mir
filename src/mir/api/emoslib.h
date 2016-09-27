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

extern "C" fortint intin_(const char *name,
                          const fortint ints[],
                          const fortfloat reals[],
                          const char *value,
                          const fortint name_len,
                          const fortint value_len);

extern "C" fortint intf_(const void *grib_in,
                         const fortint &length_in,
                         const fortfloat values_in[],
                         void *grib_out,
                         fortint &length_out,
                         fortfloat values_out[]);

extern "C" fortint intf2(const void *grib_in,
                         const fortint &length_in,
                         void *grib_out,
                         fortint &length_out);

extern "C" fortint intuvs2_(char *vort_grib_in,
                            char *div_grib_in,
                            const fortint &length_in,
                            char *u_grib_out,
                            char *v_grib_out,
                            const fortint &length_out);

extern "C" fortint intuvp2_(const void *vort_grib_in,
                            const void *div_grib_in,
                            const fortint &length_in,
                            void *u_grib_out,
                            void *v_grib_out,
                            fortint &length_out);

extern "C" fortint intvect2_(const void *u_grib_in,
                             const void *v_grib_in,
                             const fortint &length_in,
                             void *u_grib_out,
                             void *v_grib_out,
                             fortint &length_out);

extern "C" fortint intuvs_(const void *vort_grib_in,
                           const void *div_grib_in,
                           const fortint &length_in,
                           void *u_grib_out,
                           void *v_grib_out,
                           fortint &length_out);

extern "C" fortint intuvp_(const void *vort_grib_in,
                           const void *div_grib_in,
                           const fortint &length_in,
                           void *u_grib_out,
                           void *v_grib_out,
                           fortint &length_out);

extern "C" fortint intvect_(const void *u_grib_in,
                            const void *v_grib_in,
                            const fortint &length_in,
                            void *u_grib_out,
                            void *v_grib_out,
                            fortint &length_out);

extern "C" fortint iscrsz_();

extern "C" fortint ibasini_(const fortint &force);

extern "C" void intlogm_(fortint (*)(char *, fortint));

extern "C" void intlogs(emos_cb_proc proc);

extern "C" fortint areachk_(const fortfloat &we,
                            const fortfloat &ns,
                            fortfloat &north,
                            fortfloat &west,
                            fortfloat &south,
                            fortfloat &east);

extern "C" fortint emosnum_(fortint &value);

extern "C" void freecf_(const fortint &flag);

extern "C" void jvod2uv_(const fortfloat vor[],
                         const fortfloat div[],
                         const fortint &ktin,
                         fortfloat u[],
                         fortfloat v[],
                         const fortint &ktout);


extern "C" fortint jgglat_(const fortint &KLAT, fortfloat PGAUSS[]);

extern "C" void jnumgg_(const fortint &knum,
                        const char *htype,
                        fortint kpts[],
                        fortint &kret,
                        fortint htype_len);

extern "C" fortint wvqlint_(const fortint &knum,
                            const fortint numpts[],
                            const fortint &ke_w,
                            const fortint &kn_s,
                            const fortfloat &reson,
                            const fortfloat oldwave[],
                            fortfloat newwave[],
                            const fortfloat &north,
                            const fortfloat &west,
                            const fortint &kparam,
                            const fortfloat &pmiss,
                            const fortfloat &rns);

extern "C" void wv2dint_(const fortint &knum,
                         const fortint numpts[],
                         const fortint &ke_w,
                         const fortint &kn_s,
                         const fortfloat &reson,
                         const fortfloat oldwave[],
                         fortfloat newwave[],
                         const fortfloat &north,
                         const fortfloat &west,
                         const fortint &knspec, // <== What is that?
                         const fortfloat &pmiss,
                         const fortfloat &rns);

extern "C" fortint hirlam_(const fortint &l12pnt,
                           const fortfloat oldfld[],
                           const fortint &kount,
                           const fortint &kgauss,
                           const fortfloat area[],
                           const fortfloat pole[],
                           const fortfloat grid[],
                           fortfloat newfld[],
                           const fortint &ksize,
                           fortint &nlon,
                           fortint &nlat);

extern "C" fortint hirlsm_(const fortint &l12pnt,
                           const fortfloat oldfld[],
                           const fortint &kount,
                           const fortint &kgauss,
                           const fortfloat area[],
                           const fortfloat pole[],
                           const fortfloat grid[],
                           fortfloat newfld[],
                           const fortint &ksize,
                           fortint &nlon,
                           fortint &nlat);

extern "C" fortint hirlamw_(const fortint &l12pnt,
                            const fortfloat oldfldu[],
                            const fortfloat oldfldv[],
                            const fortint &kount,
                            const fortint &kgauss,
                            const fortfloat area[],
                            const fortfloat pole[],
                            const fortfloat grid[],
                            fortfloat newfldu[],
                            fortfloat newfldv[],
                            const fortint &ksize,
                            fortint &nlon,
                            fortint &nlat);

///@returns 64 (always assume "double precision")
extern "C" int emosPrecision();
