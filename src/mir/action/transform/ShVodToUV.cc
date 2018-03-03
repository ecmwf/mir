/*
 * (C) Copyright 1996- ECMWF.
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
    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().vod2uvTiming_);

    // get field properties
    data::MIRField& field = ctx.field();
    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));
    ASSERT(field.dimensions() == 2);

    size_t truncation = field.representation()->truncation();
    size_t size = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation) * 2;
    ASSERT(truncation);
    ASSERT(size);


    // get vo/d, allocate U/V
    const std::vector<double>& field_vo = field.values(0);
    const std::vector<double>& field_d = field.values(1);

    eckit::Log::debug<LibMir>() << "ShVodToUV truncation=" << truncation
                                << ", size=" << size
                                << ", values=" << field_vo.size() << std::endl;

    ASSERT(field_vo.size() == size);
    ASSERT(field_d.size() == size);

    std::vector<double> result_U(size, 0.);
    std::vector<double> result_V(size, 0.);


    // transform
    // NOTE: only type="ifs" is supprted since we don't support local spectral fields
    const int T = int(truncation);
    const int nb_coeff = int(size);
    const int nb_fields = 1;

    atlas::trans::VorDivToUV vordiv_to_UV(T, atlas::option::type("ifs"));
    ASSERT(vordiv_to_UV.truncation() == T);

    vordiv_to_UV.execute(nb_coeff, nb_fields, field_vo.data(), field_d.data(), result_U.data(), result_V.data());


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

