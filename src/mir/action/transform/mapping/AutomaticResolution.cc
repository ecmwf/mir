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


#include "mir/action/transform/mapping/AutomaticResolution.h"

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {
static ResolBuilder< AutomaticResolution > __mapping1("auto");
static ResolBuilder< AutomaticResolution > __mapping2("automatic resolution");
}


AutomaticResolution::AutomaticResolution(const param::MIRParametrisation& parametrisation) : Resol(parametrisation) {}


AutomaticResolution::~AutomaticResolution() {}


size_t AutomaticResolution::getTruncation() const {
    return 0;  // FIXME
}


size_t AutomaticResolution::getPointsPerLatitude() const {
    return 0;  // FIXME
}


void AutomaticResolution::print(std::ostream& out) const {
    ASSERT(mapping_);
    out << "AutomaticResolution[";
    mapping_->print(out);
    out << "]";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir
