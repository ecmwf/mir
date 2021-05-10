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
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Trace.h"
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
    auto& log = Log::debug();
    auto mainTimer(ctx.statistics().vod2uvTimer());

    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));


    // get field properties
    auto& field = ctx.field();

    ASSERT(0 < field.dimensions() && field.dimensions() % 2 == 0);
    size_t F = field.dimensions() / 2;

    auto T = field.representation()->truncation();
    ASSERT(T > 0);

    auto N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
    ASSERT(N > 0);


    // set input working area (avoid copies for one field pair only)
    MIRValuesVector input_vo;
    MIRValuesVector input_d;
    if (F > 1) {
        trace::Timer timer("ShVodToUV: interlacing spectra", log);

        input_vo.resize(F * N * 2);
        input_d.resize(F * N * 2);

        for (size_t f = 0, which = 0; f < F; ++f, which += 2) {
            repres::sh::SphericalHarmonics::interlace_spectra(input_vo, field.values(which + 0), T, N, f, F);
            repres::sh::SphericalHarmonics::interlace_spectra(input_d, field.values(which + 1), T, N, f, F);

            field.direct(which + 0).clear();
            field.direct(which + 1).clear();
        }
    }
    else {
        input_vo.swap(field.direct(0));
        input_d.swap(field.direct(1));
    }
    ASSERT(input_vo.size() == input_d.size());


    // set output working area
    MIRValuesVector output_U(F * N * 2, 0.);
    MIRValuesVector output_V(F * N * 2, 0.);


    // convert
    {
        trace::Timer timer("ShVodToUV: convert", log);

        atlas::trans::VorDivToUV vordiv_to_UV(int(T), options_);
        vordiv_to_UV.execute(int(N * 2), int(F), input_vo.data(), input_d.data(), output_U.data(), output_V.data());

        input_vo.clear();
        input_d.clear();
    }


    // set field values
    {
        trace::Timer timer("ShVodToUV: copying spectra", log);

        long id_u = 0;
        long id_v = 0;
        util::Wind::paramIds(parametrisation_, id_u, id_v);

        MIRValuesVector values;
        auto here_U = output_U.cbegin();
        auto here_V = output_V.cbegin();
        auto n2     = MIRValuesVector::difference_type(2 * N);

        for (size_t f = 0, which = 0; f < F; ++f, which += 2, here_U += n2, here_V += n2) {
            values.assign(here_U, here_U + n2);
            field.update(values, which);
            field.metadata(which, "paramId", id_u);

            values.assign(here_V, here_V + n2);
            field.update(values, which + 1);
            field.metadata(which + 1, "paramId", id_v);
        }
    }
}


const char* ShVodToUV::name() const {
    return "ShVodToUV";
}


static ActionBuilder<ShVodToUV> __action("transform.sh-vod-to-UV");


}  // namespace transform
}  // namespace action
}  // namespace mir
