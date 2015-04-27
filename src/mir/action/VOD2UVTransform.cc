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

#include "mir/action/VOD2UVTransform.h"

#include <iostream>
#include <complex>

#include "eckit/exception/Exceptions.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace action {


VOD2UVTransform::VOD2UVTransform(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}

VOD2UVTransform::~VOD2UVTransform() {
}


void VOD2UVTransform::print(std::ostream &out) const {
    out << "VOD2UVTransform[";
    out << "]";
}

inline double dd(double pm, double pn) {
    return -sqrt((pn*pn - pm*pm) / (4.*pn*pn - 1)) / pn;
}

inline double ss(double pm, double pn) {
    return -pm / (pn*(pn + 1));
}

void VOD2UVTransform::truncate(size_t truncation, const std::vector<double>& in, std::vector<double>& out) {
    // Truncate VO/D by one wave
    int delta = 1;
    size_t i = 0;
    size_t j = 0;

    size_t truncation_minus_1 = truncation-1;

    out.resize((truncation_minus_1+1)*(truncation_minus_1+2));

        for (size_t m = 0; m < truncation; m++) {
            for (size_t n = m ; n < truncation; n++) {
                out[i++] = in[j++];
                out[i++] = in[j++];
            }
            j += 2;
        }

    ASSERT(out.size() == j);
}


void VOD2UVTransform::execute(data::MIRField &field) const {
    ASSERT(field.dimensions() == 2);


    size_t truncation = field.representation()->truncation();
    size_t size = ((truncation+1)*(truncation+2));
    size_t truncation_minus_1 = truncation-1;
    size_t size_1 = ((truncation_minus_1+1)*(truncation_minus_1+2));


    eckit::Log::info() << "VOD2UVTransform truncation=" << truncation << ", size=" << size << ", values=" << field.values(0).size() << std::endl;

    ASSERT(field.values(0).size() == size);
    ASSERT(field.values(1).size() == size);

    std::vector<double> result_u(size);
    std::vector<double> result_v(size);

    std::vector<double> temp_vo(size_1);
    std::vector<double> temp_d(size_1);


    typedef std::vector<std::complex<double> > veccomp;
    const veccomp& vorticity = reinterpret_cast<const veccomp&>(field.values(0));
    const veccomp& divergence = reinterpret_cast<const veccomp&>(field.values(1));

    veccomp& u_component = reinterpret_cast<veccomp&>(result_u);
    veccomp& v_component = reinterpret_cast<veccomp&>(result_v);


    std::complex<double> zi (0.0,1.0);
    const double kRadiusOfTheEarth = 6.371e6; // Seriously?
    size_t        k = 0;
    size_t      imn = 0;

    /* Handle coefficients for m < truncation; n = m */
    for ( size_t j = 0 ; j < truncation ;  j++ ) {
        double zm = j ;
        double zn = zm;
        if (j) {
            u_component[k] =  (-dd(zm , zn+1.)*vorticity[imn+1] + zi*ss(zm, zn)*divergence[imn]) * kRadiusOfTheEarth ;
            v_component[k] =  ( dd(zm , zn+1.)*divergence[imn+1] + zi*ss(zm, zn)*vorticity[imn]) * kRadiusOfTheEarth ;

        } else {
            u_component[k] =  (-dd(zm, zn+1) * vorticity[imn+1]) * kRadiusOfTheEarth  ;
            v_component[k] =  ( dd(zm, zn+1) * divergence[imn+1]) * kRadiusOfTheEarth ;
        }
        ++imn;
        ++k;
        size_t  jmp = j + 1;

        /* When n < truncation - 1 */
        if (jmp < truncation - 1 ) {
            for ( int i = jmp ; i < truncation - 1 ;  i++ ) {
                zn = i;
                u_component[k] =  ( dd(zm, zn)*vorticity[imn-1] - dd(zm, zn+1)*vorticity[imn+1] + zi*ss(zm,zn)*divergence[imn]) * kRadiusOfTheEarth ;
                v_component[k] =  (-dd(zm, zn)*divergence[imn-1] + dd(zm, zn+1)*divergence[imn+1] + zi*ss(zm,zn)*vorticity[imn]) * kRadiusOfTheEarth ;
                ++k;
                ++imn;
            }
            /* When n == truncation - 1 */
            zn = truncation - 1;
            u_component[k] =  ( dd(zm, zn)*vorticity[imn-1] + zi*ss(zm, zn)*divergence[imn]) * kRadiusOfTheEarth ;
            v_component[k] =  (-dd(zm, zn)*divergence[imn-1] + zi*ss(zm, zn)*vorticity[imn]) * kRadiusOfTheEarth ;
            ++k;
            ++imn;
        }
        /* When n == truncation */
        zn = truncation;
        u_component[k] =  dd(zm, zn)*vorticity[imn-1] * kRadiusOfTheEarth ;
        v_component[k] =   -dd(zm, zn)*divergence[imn-1] * kRadiusOfTheEarth ;
        ++k;
        /* When n == truncation + 1 */
        /* IMN  = IMN + 1 + KTIN-ITOUT */
        /* KTIN-ITOUT = -1 */
        // imn = imn;
        // ++imn;
    }


    /* Handle coefficients for m = truncation */
    /* When n == truncation */
    u_component[k] = 0;
    v_component[k] = 0;
    k++;
    u_component[k] = 0;
    v_component[k] = 0;
    k++;

    eckit::Log::info() << "At end of loop: " << k << " 2k=" << 2*k << std::endl;


    ASSERT(2*k == size);

    field.values(result_u, 0);
    field.values(result_v, 1);

}


namespace {
static ActionBuilder< VOD2UVTransform > transform("transform.vod2uv");
}


}  // namespace action
}  // namespace mir

