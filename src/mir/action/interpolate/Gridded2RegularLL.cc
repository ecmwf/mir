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


#include "mir/action/interpolate/Gridded2RegularLL.h"

#include <ostream>
#include <vector>

#include "mir/key/Area.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2RegularLL::Gridded2RegularLL(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation), reference_{0, 0} {

    std::vector<double> value;
    ASSERT(parametrisation_.get("grid", value));
    ASSERT_KEYWORD_GRID_SIZE(value.size());

    increments_ = util::Increments(value[0], value[1]);

    if (key::Area::get(parametrisation_.userParametrisation(), bbox_)) {
        reference_ = {bbox_.south(), bbox_.west()};
    }

    repres::latlon::LatLon::globaliseBoundingBox(bbox_, increments_, reference_);

    Log::debug() << "Gridded2RegularLL: globalise:"
                 << "\n\t" << increments_ << "\n\t" << bbox_
                 << "\n\t"
                    "shifted in latitude? "
                 << increments_.isLatitudeShifted(bbox_)
                 << "\n\t"
                    "shifted in longitude? "
                 << increments_.isLongitudeShifted(bbox_) << std::endl;
}


Gridded2RegularLL::~Gridded2RegularLL() = default;


bool Gridded2RegularLL::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RegularLL*>(&other);
    return (o != nullptr) && (increments_ == o->increments_) && (bbox_ == o->bbox_) &&
           Gridded2GriddedInterpolation::sameAs(*o);
}


void Gridded2RegularLL::print(std::ostream& out) const {
    out << "Gridded2RegularLL["
           "increments="
        << increments_
        << ","
           "bbox="
        << bbox_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RegularLL::outputRepresentation() const {
    return new repres::latlon::RegularLL(increments_, bbox_, reference_);
}


const char* Gridded2RegularLL::name() const {
    return "Gridded2RegularLL";
}


static const ActionBuilder<Gridded2RegularLL> grid2grid("interpolate.grid2regular-ll");


}  // namespace interpolate
}  // namespace action
}  // namespace mir
