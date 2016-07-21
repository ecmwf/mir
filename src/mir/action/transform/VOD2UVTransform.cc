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


#include "mir/action/transform/VOD2UVTransform.h"

#include <complex>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/api/mir_config.h"
#include "mir/data/MIRField.h"
#include "mir/log/MIR.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/MIRStatistics.h"

#ifdef ATLAS_HAVE_TRANS
#include "transi/trans.h"
#endif


namespace mir {
namespace action {


VOD2UVTransform::VOD2UVTransform(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


VOD2UVTransform::~VOD2UVTransform() {
}


bool VOD2UVTransform::sameAs(const Action& other) const {
    const VOD2UVTransform* o = dynamic_cast<const VOD2UVTransform*>(&other);
    return o;
}


void VOD2UVTransform::print(std::ostream &out) const {
    out << "VOD2UVTransform[";
    out << "]";
}


void VOD2UVTransform::execute(context::Context & ctx) const {
    data::MIRField& field = ctx.field();
    ASSERT(field.dimensions() == 2);

    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().vod2uvTiming_);


    size_t truncation = field.representation()->truncation();
    size_t size = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation) * 2;


    const std::vector<double> &field_vo = field.values(0);
    const std::vector<double> &field_d  = field.values(1);
    std::vector<double> result_u(size, 0);
    std::vector<double> result_v(size, 0);
    ASSERT(field_vo.size() == size);
    ASSERT(field_d.size()  == size);

    eckit::Log::trace<MIR>() << "VOD2UVTransform["
                             <<  "truncation=" << truncation
                             << ",size="       << size
                             << ",values="     << field_vo.size()
                             << "]" << std::endl;

    VorDivToUV_t vod_to_UV = new_vordiv_to_UV();
    vod_to_UV.nfld   = 1;                // number of distributed fields
    vod_to_UV.ncoeff = int(size);        // number of spectral coefficients (equivalent to NSPEC2 for distributed or NSPEC2G for global)
    vod_to_UV.nsmax  = int(truncation);  // spectral resolution (T)
    vod_to_UV.rspvor = field_vo.data();  // spectral array for vorticity    DIMENSIONS(1:NFLD,1:NSPEC2)
    vod_to_UV.rspdiv = field_d.data();   // spectral array for divergence   DIMENSIONS(1:NFLD,1:NSPEC2)
    vod_to_UV.rspu   = result_u.data();  // spectral array for u*cos(theta) DIMENSIONS(1:NFLD,1:NSPEC2)
    vod_to_UV.rspv   = result_v.data();  // spectral array for v*cos(theta) DIMENSIONS(1:NFLD,1:NSPEC2)

    if (trans_vordiv_to_UV(&vod_to_UV) != TRANS_SUCCESS) {
        throw eckit::SeriousBug("trans_vordiv_to_UV: failed", Here());
    }

    field.update(result_u, 0);
    field.update(result_v, 1);

    // TODO: Find a way to get these numbers
    field.paramId(0, 131);
    field.paramId(1, 132);
}


namespace {
static ActionBuilder< VOD2UVTransform > transform("transform.vod2uv");
}


}  // namespace action
}  // namespace mir

