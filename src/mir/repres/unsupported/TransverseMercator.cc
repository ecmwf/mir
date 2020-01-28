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


#include "mir/repres/unsupported/TransverseMercator.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace repres {


TransverseMercator::TransverseMercator(const param::MIRParametrisation& parametrisation) {}


TransverseMercator::TransverseMercator() {}


TransverseMercator::~TransverseMercator() = default;


void TransverseMercator::print(std::ostream& out) const {
    out << "TransverseMercator["
        << "]";
}


void TransverseMercator::fill(grib_info& info) const {
    NOTIMP;
}


namespace {
static RepresentationBuilder<TransverseMercator> transverseMercator(
    "transverse_mercator");  // Name is what is returned by grib_api
}


}  // namespace repres
}  // namespace mir
