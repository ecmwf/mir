/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/action/transform/mapping/Auto.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {
static MappingBuilder< Auto > __mapping("auto");
}


Auto::Auto(const param::MIRParametrisation& parametrisation) : Mapping(parametrisation) {

    // defaults to linear truncation
    std::string resol = "linear";
    parametrisation.get("resol", resol);
    ASSERT(resol.length());

    if (resol == "auto") {
        resol = "linear";
    }

    eckit::ScopedPtr<Mapping> map(MappingFactory::build(resol, parametrisation));
    ASSERT(map);

    map_.swap(map);
}


Auto::~Auto() {}


size_t Auto::getTruncationFromPointsPerLatitude(const size_t& N) const {
    return map_->getTruncationFromPointsPerLatitude(N);
}


size_t Auto::getPointsPerLatitudeFromTruncation(const size_t& T) const {
    return map_->getPointsPerLatitudeFromTruncation(T);
}


void Auto::print(std::ostream& out) const {
    ASSERT(map_);
    out << "Auto[";
    map_->print(out);
    out << "]";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir
