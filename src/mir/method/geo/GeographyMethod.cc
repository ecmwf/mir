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


#include "mir/method/geo/GeographyMethod.h"

#include <ostream>


namespace mir {
namespace method {
namespace geo {


bool GeographyMethod::sameAs(const Method&) const {
    return false;
}


void GeographyMethod::print(std::ostream& os) const {
    os << "GeographyMethod[]";
}


bool GeographyMethod::validateMatrixWeights() const {
    return false;
}


}  // namespace geo
}  // namespace method
}  // namespace mir
