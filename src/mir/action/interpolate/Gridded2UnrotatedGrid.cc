// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2UnrotatedGrid.h"

#include "mir/method/Method.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"


namespace mir::action::interpolate {


util::BoundingBox Gridded2UnrotatedGrid::outputBoundingBox() const {
    repres::RepresentationHandle out(outputRepresentation());

    return method().hasCropping() ? method().getCropping() : out->domain();
}


}  // namespace mir::action::interpolate
