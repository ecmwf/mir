/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date Mar 2017


#include "mir/action/transform/mapping/Cubic.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {
static MappingBuilder< Cubic > __mapping("cubic");
}


Cubic::~Cubic() {}


size_t Cubic::getTruncationFromPointsPerLatitude(const size_t& N) const {
    ASSERT(N);

    size_t T = N - 1;
    ASSERT(T);

    return T;
}


size_t Cubic::getPointsPerLatitudeFromTruncation(const size_t& T) const {
    ASSERT(T);

    size_t N = T + 1;
    return N;
}


void Cubic::print(std::ostream& out) const {
    out << "Cubic[]";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir
