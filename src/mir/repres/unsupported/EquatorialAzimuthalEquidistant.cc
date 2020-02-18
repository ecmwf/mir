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


#include "mir/repres/unsupported/EquatorialAzimuthalEquidistant.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace repres {


EquatorialAzimuthalEquidistant::EquatorialAzimuthalEquidistant(const param::MIRParametrisation& /*parametrisation*/) {}


EquatorialAzimuthalEquidistant::EquatorialAzimuthalEquidistant() = default;


EquatorialAzimuthalEquidistant::~EquatorialAzimuthalEquidistant() = default;


void EquatorialAzimuthalEquidistant::print(std::ostream& out) const {
    out << "EquatorialAzimuthalEquidistant["
        << "]";
}


void EquatorialAzimuthalEquidistant::fill(grib_info& /*info*/) const {
    NOTIMP;
}


static RepresentationBuilder<EquatorialAzimuthalEquidistant> equatorialAzimuthalEquidistant(
    "equatorial_azimuthal_equidistant");


}  // namespace repres
}  // namespace mir
