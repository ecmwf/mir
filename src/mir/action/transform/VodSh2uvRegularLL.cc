/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/VodSh2uvRegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"


namespace mir {
namespace action {
namespace transform {


VodSh2uvRegularLL::VodSh2uvRegularLL(const param::MIRParametrisation &parametrisation):
    VodSh2uvGridded(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    grid_ = util::Increments(value[0], value[1]);

}


VodSh2uvRegularLL::~VodSh2uvRegularLL() {
}


bool VodSh2uvRegularLL::sameAs(const Action& other) const {
    const VodSh2uvRegularLL* o = dynamic_cast<const VodSh2uvRegularLL*>(&other);
    return o && (grid_ == o->grid_);
}


void VodSh2uvRegularLL::print(std::ostream &out) const {
    out << "VodSh2uvRegularLL[grib=" << grid_ << "]";
}


const repres::Representation *VodSh2uvRegularLL::outputRepresentation() const {
    return new repres::latlon::RegularLL(
                util::BoundingBox(90, 0, -90, 360 - grid_.west_east()),
                grid_);
}


namespace {
static ActionBuilder< VodSh2uvRegularLL > __action("transform.vod-sh-to-uv-regular-ll");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

