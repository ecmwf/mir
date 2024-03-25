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


#include "mir/action/interpolate/Gridded2RotatedLL.h"

#include <ostream>
#include <vector>

#include "mir/key/Area.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::action::interpolate {


Gridded2RotatedLL::Gridded2RotatedLL(const param::MIRParametrisation& parametrisation) :
    Gridded2RotatedGrid(parametrisation), reference_{0, 0} {

    std::vector<double> value;
    ASSERT(parametrisation_.get("grid", value));
    ASSERT_KEYWORD_GRID_SIZE(value.size());

    increments_ = util::Increments(value[0], value[1]);

    if (key::Area::get(parametrisation_.userParametrisation(), bbox_)) {
        reference_ = {bbox_.south(), bbox_.west()};
    }

    repres::latlon::LatLon::globaliseBoundingBox(bbox_, increments_, reference_);

    Log::debug() << "Gridded2RotatedLL: globalise:" << "\n\t" << increments_ << "\n\t" << bbox_
                 << "\n\t"
                    "shifted in latitude? "
                 << increments_.isLatitudeShifted(bbox_)
                 << "\n\t"
                    "shifted in longitude? "
                 << increments_.isLongitudeShifted(bbox_) << std::endl;
}


Gridded2RotatedLL::~Gridded2RotatedLL() = default;


bool Gridded2RotatedLL::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedLL*>(&other);
    return (o != nullptr) && (increments_ == o->increments_) && (bbox_ == o->bbox_) && Gridded2RotatedGrid::sameAs(*o);
}


void Gridded2RotatedLL::print(std::ostream& out) const {
    out << "Gridded2RotatedLL["
           "increments="
        << increments_
        << ","
           "bbox="
        << bbox_
        << ","
           "rotation="
        << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedLL::outputRepresentation() const {
    return new repres::latlon::RotatedLL(increments_, rotation(), bbox_, reference_);
}


const char* Gridded2RotatedLL::name() const {
    return "Gridded2RotatedLL";
}


static const ActionBuilder<Gridded2RotatedLL> grid2grid("interpolate.grid2rotated-regular-ll");


}  // namespace mir::action::interpolate
