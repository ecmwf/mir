/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/ShVodTouvRegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvRegularLL::ShVodTouvRegularLL(const param::MIRParametrisation &parametrisation):
    ShVodTouvGridded(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

}


ShVodTouvRegularLL::~ShVodTouvRegularLL() {
}


bool ShVodTouvRegularLL::sameAs(const Action& other) const {
    const ShVodTouvRegularLL* o = dynamic_cast<const ShVodTouvRegularLL*>(&other);
    return o && (increments_ == o->increments_);
}


void ShVodTouvRegularLL::print(std::ostream &out) const {
    out << "ShVodTouvRegularLL[increments=" << increments_ << "]";
}

const char* ShVodTouvRegularLL::name() const {
    return "ShVodTouvRegularLL";
}

const repres::Representation *ShVodTouvRegularLL::outputRepresentation() const {

    if (!increments_.isPeriodic()) {
        throw eckit::UserError("Spectral transforms only support periodic regular grids", Here());
    }

    // use (non-shifted) global bounding box
    util::BoundingBox bbox;
    increments_.globaliseBoundingBox(bbox, false, false);

    return new repres::latlon::RegularLL(bbox, increments_);
}


namespace {
static ActionBuilder< ShVodTouvRegularLL > __action("transform.sh-vod-to-uv-regular-ll");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

