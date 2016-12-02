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
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/util/MIRStatistics.h"


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


inline double dd(double pm, double pn) {
    return -sqrt((pn * pn - pm * pm) / (4.*pn * pn - 1)) / pn;
}


inline double ss(double pm, double pn) {
    return -pm / (pn * (pn + 1));
}


void Vod2uv::execute(context::Context & ctx) const {
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

    std::vector<double> result_u(size, 0);
    std::vector<double> result_v(size, 0);

    std::vector<double> temp_vo;
    std::vector<double> temp_d;


    repres::sh::SphericalHarmonics::truncate(truncation, truncation - 1, field_vo, temp_vo);
    repres::sh::SphericalHarmonics::truncate(truncation, truncation - 1, field_d, temp_d);


    typedef std::vector<std::complex<double> > veccomp;
    const veccomp &vorticity = reinterpret_cast<const veccomp &>(temp_vo);
    const veccomp &divergence = reinterpret_cast<const veccomp &>(temp_d);

    veccomp &u_component = reinterpret_cast<veccomp &>(result_u);
    veccomp &v_component = reinterpret_cast<veccomp &>(result_v);


    std::complex<double> zi(0.0, 1.0);
    const double kRadiusOfTheEarth = atlas::util::Earth::radiusInMeters();
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

    field.update(result_u, 0);
    field.update(result_v, 1);


    // configure paramIds for u/v
    long id_u = 131;
    long id_v = 132;
    parametrisation_.get("transform.vod2uv.u", id_u);
    parametrisation_.get("transform.vod2uv.v", id_v);

    field.metadata(0, "paramId", id_u);
    field.metadata(1, "paramId", id_v);
}


namespace {
static ActionBuilder< Vod2uv > transform("transform.vod2uv");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

