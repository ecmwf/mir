/*
 * (C) Copyright 1996-2017 ECMWF.
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


#include "mir/action/transform/ShVodToUV.h"

#include <complex>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/context/Context.h"
#include "mir/action/transform/TransInitor.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/MIRStatistics.h"
#include "mir/api/Atlas.h"


namespace mir {
namespace action {
namespace transform {


ShVodToUV::ShVodToUV(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


ShVodToUV::~ShVodToUV() {
}


bool ShVodToUV::sameAs(const Action& other) const {
    const ShVodToUV* o = dynamic_cast<const ShVodToUV*>(&other);
    return o;
}


void ShVodToUV::print(std::ostream &out) const {
    out << "ShVodToUV["
        << "]";
}


void ShVodToUV::execute(context::Context & ctx) const {
    data::MIRField& field = ctx.field();

    ASSERT(field.dimensions() == 2);

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().vod2uvTiming_);


    size_t truncation = field.representation()->truncation();
    size_t size = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation) * 2;


    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));

    const std::vector<double> &field_vo = field.values(0);
    const std::vector<double> &field_d = field.values(1);

    eckit::Log::debug<LibMir>() << "ShVodToUV truncation=" << truncation
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
    int err = trans_vordiv_to_UV(&vod_to_UV);

    if (err != TRANS_SUCCESS) {
        std::ostringstream oss;
        oss << "trans_vordiv_to_UV failed: " << trans_error_msg(err);
        throw eckit::SeriousBug(oss.str());
    }


    // configure paramIds for U/V
    const eckit::Configuration& config = LibMir::instance().configuration();
    const long id_u = config.getLong("parameter-id-u", 131);
    const long id_v = config.getLong("parameter-id-v", 132);


    field.update(result_U, 0);
    field.metadata(0, "paramId", id_u);

    field.update(result_V, 1);
    field.metadata(1, "paramId", id_v);

}

const char* ShVodToUV::name() const {
    return "ShVodToUV";
}


namespace {
static ActionBuilder< ShVodToUV > __action("transform.sh-vod-to-UV");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

