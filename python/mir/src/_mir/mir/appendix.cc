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


#include "mir/appendix.h"

#include <algorithm>
#include <iterator>

#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/GridBox.h"


namespace mir::appendix {


std::vector<double> grid_box_areas(const std::string& gridspec) {
    repres::RepresentationHandle repres(repres::RepresentationFactory::build(param::GridSpecParametrisation(gridspec)));
    const auto boxes = repres->gridBoxes();

    std::vector<double> areas;
    areas.reserve(boxes.size());
    std::transform(boxes.begin(), boxes.end(), std::back_inserter(areas), [](const auto& box) { return box.area(); });

    return areas;
}


}  // namespace mir::appendix
