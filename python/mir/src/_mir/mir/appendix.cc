// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
