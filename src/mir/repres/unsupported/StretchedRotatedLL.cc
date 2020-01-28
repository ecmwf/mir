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


#include "mir/repres/unsupported/StretchedRotatedLL.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace repres {


StretchedRotatedLL::StretchedRotatedLL(const param::MIRParametrisation &parametrisation) {
}


StretchedRotatedLL::StretchedRotatedLL() {
}


StretchedRotatedLL::~StretchedRotatedLL() = default;


void StretchedRotatedLL::print(std::ostream &out) const {
    out << "StretchedRotatedLL["
        << "]";
}


void StretchedRotatedLL::fill(grib_info &info) const  {
    NOTIMP;
}


namespace {
static RepresentationBuilder<StretchedRotatedLL> stretchedRotatedLL("stretched_rotated_ll"); // Name is what is returned by grib_api
}


}  // namespace repres
}  // namespace mir

