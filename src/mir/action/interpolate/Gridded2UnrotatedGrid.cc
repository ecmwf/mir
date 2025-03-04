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
