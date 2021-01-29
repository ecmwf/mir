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


#include "mir/action/interpolate/Gridded2RotatedGrid.h"

#include <vector>

#include "mir/method/Method.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2RotatedGrid::Gridded2RotatedGrid(const param::MIRParametrisation& parametrisation) :
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));

    ASSERT_KEYWORD_ROTATION_SIZE(value.size());
    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedGrid::~Gridded2RotatedGrid() = default;


const util::Rotation& Gridded2RotatedGrid::rotation() const {
    return rotation_;
}


bool Gridded2RotatedGrid::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2RotatedGrid*>(&other);
    return (o != nullptr) && (rotation_ == o->rotation_) && Gridded2GriddedInterpolation::sameAs(other);
}


util::BoundingBox Gridded2RotatedGrid::outputBoundingBox() const {
    repres::RepresentationHandle out(outputRepresentation());

    const auto& bbox(method().hasCropping() ? method().getCropping() : out->domain());

    bbox_ = rotation_.boundingBox(bbox);
    return bbox_;
}


}  // namespace interpolate
}  // namespace action
}  // namespace mir
