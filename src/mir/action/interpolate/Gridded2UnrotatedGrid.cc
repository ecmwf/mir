/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/interpolate/Gridded2UnrotatedGrid.h"

#include "mir/action/misc/AreaCropper.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2UnrotatedGrid::~Gridded2UnrotatedGrid() = default;


const util::BoundingBox& Gridded2UnrotatedGrid::croppingBoundingBox() const {
    repres::RepresentationHandle out(outputRepresentation());

    return method().hasCropping() ? method().getCropping()
                                  : out->boundingBox();
}


void Gridded2UnrotatedGrid::cropToInput(context::Context& ctx, const repres::Representation& in) const {

    // * only crop if input is not global
    // * output representation comes from Context
    if (!in.isGlobal()) {
        AreaCropper cropper(parametrisation_, in.boundingBox());
        cropper.execute(ctx);
    }
}


}  // namespace interpolate
}  // namespace action
}  // namespace mir

