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


#include "mir/action/transform/Vod2uv.h"

#include <complex>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "atlas/util/Constants.h"
#include "mir/action/context/Context.h"
#include "mir/action/transform/TransInitor.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/MIRStatistics.h"

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"

#define TRANS_CHECK( CALL ) do {\
  int errcode = CALL;\
  if( errcode != TRANS_SUCCESS) {\
    printf("ERROR: %s failed @%s:%d:\n%s\n",#CALL,__FILE__,__LINE__,trans_error_msg(errcode));\
    abort();\
  }\
} while(0)

#endif


namespace mir {
namespace action {
namespace transform {


Vod2uv::Vod2uv(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


Vod2uv::~Vod2uv() {
}


bool Vod2uv::sameAs(const Action& other) const {
    const Vod2uv* o = dynamic_cast<const Vod2uv*>(&other);
    return o;
}


void Vod2uv::print(std::ostream &out) const {
    out << "Vod2uv[";
    out << "]";
}


void Vod2uv::execute(context::Context & ctx) const {
#ifdef ATLAS_HAVE_TRANS
    data::MIRField& field = ctx.field();

    ASSERT(field.dimensions() == 2);

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().vod2uvTiming_);


    size_t truncation = field.representation()->truncation();
    size_t size = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation) * 2;


    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));

    const std::vector<double> &field_vo = field.values(0);
    const std::vector<double> &field_d = field.values(1);

    eckit::Log::debug<LibMir>() << "Vod2uv truncation=" << truncation
                                << ", size=" << size
                                << ", values=" << field_vo.size() << std::endl;

    ASSERT(field_vo.size() == size);
    ASSERT(field_d.size() == size);

    std::vector<double> result_U(size, 0);
    std::vector<double> result_V(size, 0);

    struct VorDivToUV_t vod_to_UV = new_vordiv_to_UV();
    vod_to_UV.nfld   = 1;
    vod_to_UV.ncoeff = static_cast<int>(size);        // number of coefficients
    vod_to_UV.nsmax  = static_cast<int>(truncation);  // spectral resolution (T)
    vod_to_UV.rspvor = field_vo.data();
    vod_to_UV.rspdiv = field_d.data();
    vod_to_UV.rspu   = result_U.data();
    vod_to_UV.rspv   = result_V.data();


    TransInitor::instance(); // Will init trans if needed
    TRANS_CHECK( trans_vordiv_to_UV(&vod_to_UV) );


    field.update(result_U, 0);
    field.update(result_V, 1);


    // configure paramIds for U/V
    long id_u = 131;
    long id_v = 132;
    parametrisation_.get("transform.vod2uv.u", id_u);
    parametrisation_.get("transform.vod2uv.v", id_v);

    field.metadata(0, "paramId", id_u);
    field.metadata(1, "paramId", id_v);
#else
    throw eckit::SeriousBug("Spherical harmonics transforms are not supported. "
                            "Please recompile ATLAS with TRANS support enabled.");
#endif
}


namespace {
static ActionBuilder< Vod2uv > transform("transform.vod2UV");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

