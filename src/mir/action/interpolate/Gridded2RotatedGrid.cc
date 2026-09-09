// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2RotatedGrid.h"

#include <vector>

#include "mir/method/Method.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RotatedGrid::Gridded2RotatedGrid(const param::MIRParametrisation& param) : Gridded2GriddedInterpolation(param) {

    std::vector<double> value;
    ASSERT(parametrisation().userParametrisation().get("rotation", value));

    ASSERT_KEYWORD_ROTATION_SIZE(value.size());
    rotation_ = util::Rotation(value[0], value[1]);
}


const util::Rotation& Gridded2RotatedGrid::rotation() const {
    return rotation_;
}


bool Gridded2RotatedGrid::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedGrid*>(&other);
    return (o != nullptr) && (rotation_ == o->rotation_) && Gridded2GriddedInterpolation::sameAs(other);
}


util::BoundingBox Gridded2RotatedGrid::outputBoundingBox() const {
    repres::RepresentationHandle out(outputRepresentation());

    const auto& bbox(method().hasCropping() ? method().getCropping() : out->domain());

    bbox_ = rotation_.boundingBox(bbox);
    return bbox_;
}


}  // namespace mir::action::interpolate
