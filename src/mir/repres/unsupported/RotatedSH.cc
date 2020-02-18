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


#include "mir/repres/unsupported/RotatedSH.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace repres {


RotatedSH::RotatedSH(const param::MIRParametrisation& parametrisation) : SphericalHarmonics(parametrisation) {}

RotatedSH::~RotatedSH() = default;


void RotatedSH::print(std::ostream& out) const {
    out << "RotatedSH["
        << "]";
}


void RotatedSH::makeName(std::ostream& /*out*/) const {
    NOTIMP;
}

bool RotatedSH::sameAs(const Representation& /*other*/) const {
    NOTIMP;
}

void RotatedSH::fill(grib_info& /*info*/) const {
    NOTIMP;
}


static RepresentationBuilder<RotatedSH> rotatedSH("rotated_sh");  // Name is what is returned by grib_api


}  // namespace repres
}  // namespace mir
