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


#include "mir/repres/unsupported/StretchedLL.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace repres {


StretchedLL::StretchedLL(const param::MIRParametrisation& /*parametrisation*/) {}


StretchedLL::StretchedLL() = default;


StretchedLL::~StretchedLL() = default;


void StretchedLL::print(std::ostream& out) const {
    out << "StretchedLL["
        << "]";
}


void StretchedLL::fill(grib_info& /*info*/) const {
    NOTIMP;
}


static RepresentationBuilder<StretchedLL> stretchedLL("stretched_ll");


}  // namespace repres
}  // namespace mir
