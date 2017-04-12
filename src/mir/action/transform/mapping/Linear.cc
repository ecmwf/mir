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


#include "mir/action/transform/mapping/Linear.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {
static MappingBuilder< Linear > __mapping("linear");
}


Linear::~Linear() {}


size_t Linear::getTruncationFromPointsPerLatitude(const size_t& N) const {
    ASSERT(N);

    size_t T = 2 * N - 1;
    return T;
}


size_t Linear::getPointsPerLatitudeFromTruncation(const size_t& T) const {
    ASSERT(T);

    size_t N = size_t(double(T + 1)/2.);
    return N;
}


void Linear::print(std::ostream& out) const {
    out << "Linear[]";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir
