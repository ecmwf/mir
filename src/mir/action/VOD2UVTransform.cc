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
    const std::vector<double> &values = field.values();
    std::vector<double> result;

    size_t truncation = field.representation()->truncation();
    size_t size = ((truncation+1)*(truncation+2))/2;
    size_t half = values.size()/4;
    ASSERT(half == size*2);



    std::vector<std::complex<double> > vorticity;
    vorticity.reserve(size);
    for(size_t j = 0, k = 0; j < half; j++) {
        double re = values[k++];
        double im = values[k++];
        vorticity.push_back(std::complex<double>(re, im));
    }

    std::vector<std::complex<double> > divergence;
    divergence.reserve(size);
    for(size_t j = 0, k = half; j < half; j++) {
        double re = values[k++];
        double im = values[k++];
        divergence.push_back(std::complex<double>(re, im));
    }

    std::vector<std::complex<double> > u_component;
    u_component.reserve(size);
    std::vector<std::complex<double> > v_component;
    v_component.reserve(size);

    std::complex<double> zi (0.0,1.0);
    const double kRadiusOfTheEarth = 6.371e6; // Seriously?
    long        k = 0;
    long      imn = 0;

    /* Handle coefficients for m < truncation; n = m */
    for ( size_t j = 0 ; j < truncation ;  j++ ) {
        double zm = j ;
        double zn = zm;
        if (j) {
            u_component.push_back( (-dd(zm,zn+1.)*vorticity[imn+1] + zi*ss(zm,zn)*divergence[imn]) * kRadiusOfTheEarth );
            v_component.push_back( ( dd(zm,zn+1.)*divergence[imn+1] + zi*ss(zm,zn)*vorticity[imn]) * kRadiusOfTheEarth );

        } else {
            u_component.push_back( (-dd(zm,zn+1) * vorticity[imn+1]) * kRadiusOfTheEarth ) ;
            v_component.push_back( ( dd(zm,zn+1) * divergence[imn+1]) * kRadiusOfTheEarth );
        }
        ++imn;
        ++k;
        int  jmp = j + 1;

        /* When n < truncation - 1 */
        if (jmp < truncation - 1 ) {
            for ( int i = jmp ; i < truncation - 1 ;  i++ ) {
                zn = i;
                u_component.push_back( ( dd(zm,zn)*vorticity[imn-1] - dd(zm,zn+1)*vorticity[imn+1] + zi*ss(zm,zn)*divergence[imn]) * kRadiusOfTheEarth );
                v_component.push_back( (-dd(zm,zn)*divergence[imn-1] + dd(zm,zn+1)*divergence[imn+1] + zi*ss(zm,zn)*vorticity[imn]) * kRadiusOfTheEarth );
                ++k;
                ++imn;
            }
            /* When n == truncation - 1 */
            zn = truncation - 1;
            u_component.push_back( ( dd(zm,zn)*vorticity[imn-1] + zi*ss(zm,zn)*divergence[imn]) * kRadiusOfTheEarth );
            v_component.push_back( (-dd(zm,zn)*divergence[imn-1] + zi*ss(zm,zn)*vorticity[imn]) * kRadiusOfTheEarth );
            ++k;
            ++imn;
        }
        /* When n == truncation */
        zn = truncation;
        u_component.push_back( ( dd(zm,zn)*vorticity[imn-1]) * kRadiusOfTheEarth );
        v_component.push_back( ( -dd(zm,zn)*divergence[imn-1]) * kRadiusOfTheEarth );
        ++k;
        /* When n == truncation + 1 */
        /* IMN  = IMN + 1 + KTIN-ITOUT */
        /* KTIN-ITOUT = -1 */
        imn = imn;
    }

    /* Handle coefficients for m = truncation */
    /* When n == truncation */
    u_component.push_back(0);
    v_component.push_back(0);
    u_component.push_back(0);
    v_component.push_back(0);

    ASSERT(u_component.size() == size && v_component.size() == size);


}


namespace {
static ActionBuilder< VOD2UVTransform > transform("transform.vod2uv");
}


}  // namespace action
}  // namespace mir

