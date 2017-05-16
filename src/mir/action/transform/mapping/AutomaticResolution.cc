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
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


AutomaticResolution::AutomaticResolution(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {

    std::string map;
    parametrisation_.get("spectral-mapping", map);

    mapping_.reset(action::transform::mapping::MappingFactory::build(map));
    ASSERT(mapping_);
}


bool AutomaticResolution::get(const std::string &name, long &value) const {
    ASSERT(name == "truncation");

    if (parametrisation_.has("griddef")) {
        // TODO: this is temporary
        value = 63L;
        return true;
    }


    long Ni = 0;  // points-per-latitude

    if (parametrisation_.has("user.grid")) {}
    if (parametrisation_.has("user.reduced")) {}
    if (parametrisation_.has("user.regular")) {}
    if (parametrisation_.has("user.octahedral")) {}
    if (parametrisation_.has("user.pl")) {}
    if (parametrisation_.has("user.gridname")) {}
    if (parametrisation_.has("user.griddef")) {}


    ASSERT(Ni > 0);
    return mapping_->getTruncationFromPointsPerLatitude(Ni);
}


bool AutomaticResolution::get(const std::string& name, size_t& value) const {
    long T;
    if (get(name, T)) {
        value = size_t(T);
        return true;
    }
    return false;
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
