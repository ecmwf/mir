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


void VOD2UVTransform::execute(data::MIRField &field) const {
    ASSERT(field.dimensions() == 2);


    size_t truncation = field.representation()->truncation();
    size_t size = ((truncation+1)*(truncation+2));

    ASSERT(field.values(0).size() == size);
    ASSERT(field.values(1).size() == size);

    std::vector<double> result_u(size);
    std::vector<double> result_v(size);

    typedef std::vector<std::complex<double> > veccomp;

    const veccomp& vorticity = reinterpret_cast<const veccomp&>(field.values(0));
    const veccomp& divergence = reinterpret_cast<const veccomp&>(field.values(1));

     veccomp& u_component = reinterpret_cast<veccomp&>(result_u);
     veccomp& v_component = reinterpret_cast<veccomp&>(result_v);


    std::complex<double> zi (0.0,1.0);
    const double kRadiusOfTheEarth = 6.371e6; // Seriously?
    long        k = 0;
    long      imn = 0;

    size_t i = 0;

    /* Handle coefficients for m < truncation; n = m */
    for ( size_t j = 0 ; j < truncation ;  j++ ) {
        double zm = j ;
        double zn = zm;
        if (j) {
            u_component[i++] =  (-dd(zm , zn+1.)*vorticity[imn+1] + zi*ss(zm, zn)*divergence[imn]) * kRadiusOfTheEarth ;
            v_component[i++] =  ( dd(zm , zn+1.)*divergence[imn+1] + zi*ss(zm, zn)*vorticity[imn]) * kRadiusOfTheEarth ;

        } else {
            u_component[i++] =  (-dd(zm, zn+1) * vorticity[imn+1]) * kRadiusOfTheEarth  ;
            v_component[i++] =  ( dd(zm, zn+1) * divergence[imn+1]) * kRadiusOfTheEarth ;
        }
        ++imn;
        ++k;
        int  jmp = j + 1;

        /* When n < truncation - 1 */
        if (jmp < truncation - 1 ) {
            for ( int i = jmp ; i < truncation - 1 ;  i++ ) {
                zn = i;
                u_component[i++] =  ( dd(zm, zn)*vorticity[imn-1] - dd(zm, zn+1)*vorticity[imn+1] + zi*ss(zm,zn)*divergence[imn]) * kRadiusOfTheEarth ;
                v_component[i++] =  (-dd(zm, zn)*divergence[imn-1] + dd(zm, zn+1)*divergence[imn+1] + zi*ss(zm,zn)*vorticity[imn]) * kRadiusOfTheEarth ;
                ++k;
                ++imn;
            }
            /* When n == truncation - 1 */
            zn = truncation - 1;
            u_component[i++] =  ( dd(zm, zn)*vorticity[imn-1] + zi*ss(zm, zn)*divergence[imn]) * kRadiusOfTheEarth ;
            v_component[i++] =  (-dd(zm, zn)*divergence[imn-1] + zi*ss(zm, zn)*vorticity[imn]) * kRadiusOfTheEarth ;
            ++k;
            ++imn;
        }
        /* When n == truncation */
        zn = truncation;
        u_component[i++] =  dd(zm, zn)*vorticity[imn-1] * kRadiusOfTheEarth ;
        v_component[i++] =   -dd(zm, zn)*divergence[imn-1] * kRadiusOfTheEarth ;
        ++k;
        /* When n == truncation + 1 */
        /* IMN  = IMN + 1 + KTIN-ITOUT */
        /* KTIN-ITOUT = -1 */
        // imn = imn;
    }

    /* Handle coefficients for m = truncation */
    /* When n == truncation */
    u_component[i++] = 0;
    v_component[i++] = 0;
    u_component[i++] = 0;
    v_component[i++] = 0;

    ASSERT(i == size);

    field.values(result_u, 0);
    field.values(result_v, 1);

}


namespace {
static ActionBuilder< VOD2UVTransform > transform("transform.vod2uv");
}


}  // namespace action
}  // namespace mir

