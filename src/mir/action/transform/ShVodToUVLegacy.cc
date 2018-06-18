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


#include "mir/action/transform/ShVodToUVLegacy.h"

#include <complex>
#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "atlas/trans/VorDivToUV.h"

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


inline double dd(double pm, double pn) {
    return -sqrt((pn * pn - pm * pm) / (4.*pn * pn - 1)) / pn;
}


inline double ss(double pm, double pn) {
    return -pm / (pn * (pn + 1));
}


ShVodToUVLegacy::ShVodToUVLegacy(const param::MIRParametrisation& parametrisation):
    Action(parametrisation) {
#if 0
    bool plus = parametrisation.has("vod2UV-legacy-plus-one-wave");
    bool minus = parametrisation.has("vod2UV-legacy-minus-one-wave");

    ASSERT(plus != minus);
    plusOneWave_ = plus;
#else
    plusOneWave_ = false;
#endif
}


ShVodToUVLegacy::~ShVodToUVLegacy() {
}


bool ShVodToUVLegacy::sameAs(const Action& other) const {
    const ShVodToUVLegacy* o = dynamic_cast<const ShVodToUVLegacy*>(&other);
    return o;
}


void ShVodToUVLegacy::print(std::ostream &out) const {
    out << "ShVodToUVLegacy["
        <<  "plusOneWave=" << plusOneWave_
        << "]";
}


void ShVodToUVLegacy::execute(context::Context & ctx) const {
    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().vod2uvTiming_);

    // get field properties
    data::MIRField& field = ctx.field();
    ASSERT(sizeof(std::complex<double>) == 2 * sizeof(double));
    ASSERT(field.dimensions() == 2);

    size_t truncation = field.representation()->truncation();
    size_t size = repres::sh::SphericalHarmonics::number_of_complex_coefficients(truncation) * 2;
    ASSERT(truncation > 1);
    ASSERT(size);


    // get vo/d, allocate U/V
    const MIRValuesVector& field_vo = field.values(0);
    const MIRValuesVector& field_d = field.values(1);

    eckit::Log::debug<LibMir>() << "ShVodToUVLegacy truncation=" << truncation
                                << ", size=" << size
                                << ", values=" << field_vo.size() << std::endl;

    ASSERT(field_vo.size() == size);
    ASSERT(field_d.size() == size);

    MIRValuesVector result_U(size, 0.);
    MIRValuesVector result_V(size, 0.);


    // transform

    MIRValuesVector temp_vo;
    MIRValuesVector temp_d;

    size_t to = plusOneWave_? truncation + 1 : truncation - 1;
    repres::sh::SphericalHarmonics::truncate(truncation, to, field_vo, temp_vo);
    repres::sh::SphericalHarmonics::truncate(truncation, to, field_d, temp_d);


    using veccomp = data::MIRValuesVectorT<std::complex<double>>;
    auto& vorticity = reinterpret_cast<const veccomp&>(temp_vo);
    auto& divergence = reinterpret_cast<const veccomp&>(temp_d);

    auto& u_component = reinterpret_cast<veccomp&>(result_U);
    auto& v_component = reinterpret_cast<veccomp&>(result_V);


    // ref. libemos/gribex/vod2uv.F

    std::complex<double> zi(0.0, 1.0);
    const double kRadiusOfTheEarth = atlas::util::Earth::radius();
    size_t k = 0;
    size_t imn = 0;

    size_t count = truncation;


    for (size_t j = 0 ; j < count ;  j++) {
        double zm = j;
        double zn = zm;

        double ddmn1 = dd(zm, zn + 1.);
        double ssmn = ss(zm, zn);
        if (j) {
            u_component[k] = (-ddmn1 * vorticity[imn + 1] + zi * ssmn * divergence[imn]) * kRadiusOfTheEarth;
            v_component[k] = ( ddmn1 * divergence[imn + 1] + zi * ssmn * vorticity[imn]) * kRadiusOfTheEarth;

        } else {
            u_component[k] = (-ddmn1 * vorticity[imn + 1]) * kRadiusOfTheEarth ;
            v_component[k] = ( ddmn1 * divergence[imn + 1]) * kRadiusOfTheEarth;
        }

        imn++;
        k++;
        size_t  jmp = j + 1;

        if (jmp < count - 1) {
            for (size_t i = jmp; i < count - 1;  i++) {
                zn = i;

                double ddzmn = dd(zm, zn);
                double ddmn1 = dd(zm, zn + 1.);
                double ssmn = ss(zm, zn);
                u_component[k] =  ( ddzmn * vorticity[imn - 1] - ddmn1 * vorticity[imn + 1] + zi * ssmn * divergence[imn]) * kRadiusOfTheEarth;
                v_component[k] =  (-ddzmn * divergence[imn - 1] + ddmn1 * divergence[imn + 1] + zi * ssmn * vorticity[imn]) * kRadiusOfTheEarth;
                k++;
                imn++;
            }

            zn = count - 1;
            double ddzmn = dd(zm, zn);
            double ssmn = ss(zm, zn);
            u_component[k] =  ( ddzmn * vorticity[imn - 1] + zi * ssmn * divergence[imn]) * kRadiusOfTheEarth;
            v_component[k] =  (-ddzmn * divergence[imn - 1] + zi * ssmn * vorticity[imn]) * kRadiusOfTheEarth;
            k++;
            imn++;
        }

        zn = count;
        double ddzmn = dd(zm, zn);
        u_component[k] =  ddzmn * vorticity[imn - 1] * kRadiusOfTheEarth;
        v_component[k] =  -ddzmn * divergence[imn - 1] * kRadiusOfTheEarth;
        k++;

    }

    while (2 * k < size) {
        u_component[k] = 0;
        v_component[k] = 0;
        k++;
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

const char* ShVodToUVLegacy::name() const {
    return "ShVodToUVLegacy";
}


namespace {
static ActionBuilder< ShVodToUVLegacy > __action1("transform.sh-vod-to-UV-legacy-minus-one-wave");
static ActionBuilder< ShVodToUVLegacy > __action2("transform.sh-vod-to-UV-legacy-plus-one-wave");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

