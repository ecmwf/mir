/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/ShVodToUV.h"

#include <complex>
#include <ostream>
#include <sstream>

#include "atlas/trans/VorDivToUV.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Wind.h"


namespace mir {
namespace action {
namespace transform {


ShVodToUV::ShVodToUV(const param::MIRParametrisation& parametrisation) : Action(parametrisation) {

    // use the 'local' spectral transforms
    std::string type = "local";
    parametrisation.get("atlas-trans-type", type);

    if (!atlas::trans::Trans::hasBackend(type)) {
        std::ostringstream msg;
        msg << "ShVodToUV: Atlas/Trans spectral transforms type '" << type << "' not supported, available types are: ";
        atlas::trans::Trans::listBackends(msg);
        Log::error() << msg.str() << std::endl;
        throw exception::UserError(msg.str());
    }

    options_.set(atlas::option::type(type));
}


ShVodToUV::~ShVodToUV() = default;


bool ShVodToUV::sameAs(const Action& other) const {
    auto o = dynamic_cast<const ShVodToUV*>(&other);
    return (o != nullptr);
}


void ShVodToUV::print(std::ostream& out) const {
    out << "ShVodToUV[type=" << options_.getString("type") << "]";
}


void ShVodToUV::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().vod2uvTimer());

    // get field properties
    data::MIRField& field = ctx.field();
    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));
    ASSERT(field.dimensions() == 2);

    size_t truncation = field.representation()->truncation();
    size_t size       = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation) * 2;
    ASSERT(truncation);
    ASSERT(size);


    // get vo/d, allocate U/V
    const MIRValuesVector& field_vo = field.values(0);
    const MIRValuesVector& field_d  = field.values(1);

    Log::debug() << "ShVodToUV truncation=" << truncation << ", size=" << size << ", values=" << field_vo.size()
                 << std::endl;

    if (field_vo.size() != field_d.size()) {
        Log::error() << "ShVodToUV: input fields have different truncation: " << field_vo.size() << "/"
                     << field_d.size() << std::endl;
        ASSERT(field_vo.size() == field_d.size());
    }

    MIRValuesVector result_U(size, 0.);
    MIRValuesVector result_V(size, 0.);


    // transform
    const int T         = int(truncation);
    const int nb_coeff  = int(size);
    const int nb_fields = 1;

    atlas::trans::VorDivToUV vordiv_to_UV(T, options_);
    ASSERT(vordiv_to_UV.truncation() == T);

    vordiv_to_UV.execute(nb_coeff, nb_fields, field_vo.data(), field_d.data(), result_U.data(), result_V.data());


    // configure paramIds for U/V
    long id_u = 0;
    long id_v = 0;
    util::Wind::paramIds(parametrisation_, id_u, id_v);


    field.update(result_U, 0);
    field.metadata(0, "paramId", id_u);

    field.update(result_V, 1);
    field.metadata(1, "paramId", id_v);
}

const char* ShVodToUV::name() const {
    return "ShVodToUV";
}


static ActionBuilder<ShVodToUV> __action("transform.sh-vod-to-UV");


}  // namespace transform
}  // namespace action
}  // namespace mir
