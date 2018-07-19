/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>

#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "mir/api/Atlas.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/util/Domain.h"


namespace mir {
namespace tests {
namespace unit {


CASE("Test number of points representation <=> grid") {
    auto& log = eckit::Log::info();

    using repres::latlon::RegularLL;
    using namedgrids::NamedGrid;


   repres::RepresentationHandle representations[] = {
       new RegularLL(util::Increments(1., 1.)),
       new RegularLL(util::Increments(1., 1.), util::BoundingBox(90, 0, 90, 360)),
       NamedGrid::lookup("O16").representation(),
       NamedGrid::lookup("O1280").representation(),
    };

    for (const auto& repres : representations) {
        size_t n1 = repres->numberOfPoints();
        log << "#=" << n1 << "\tfrom " << *repres << std::endl;

        auto grid = repres->atlasGrid();
        size_t n2 = grid.size();
        log << "#=" << n2 << "\tfrom " << grid.spec() << std::endl;

        EXPECT(n1 == n2);
    }
}


CASE("Test number of points representation <=> cropped grid") {
    auto& log = eckit::Log::info();

    using repres::latlon::RegularLL;

    const util::Domain domains[] = {
        {  90,    0,  90,  360 },  // North pole
        {  90,    0,  89,  360 },
        {   0,    0,   0,  360 },
        { -89,    0, -90,  360 },
        { -90,    0, -90,  360 },  // South pole
        {  90,    0, -90,    0 },  // Greenwhich
        {  90,    0, -90,    1 },
        {  90,  180, -90,  180 },  // date line
        {  90, -180, -90, -180 },  // date line
        {  90,   -1, -90,    0 },
        {  90,   -1, -90,   -1 },
    };

    for (const auto& dom : domains) {
        ASSERT(!dom.isGlobal());

        repres::RepresentationHandle repres(new RegularLL(util::Increments(1., 1.)));
        repres::RepresentationHandle represCropped(repres->croppedRepresentation(dom));
        size_t n1 = represCropped->numberOfPoints();
        log << "#=" << n1 << "\tfrom " << *represCropped << std::endl;

        auto grid = repres->atlasGrid();
        ASSERT(grid.domain().global());

        auto gridCropped = atlas::Grid(grid, dom);
        ASSERT(!gridCropped.domain().global());

        size_t n2 = gridCropped.size();
        log << "#=" << n2 << "\tfrom " << gridCropped.spec() << std::endl;

        EXPECT(n1 == n2);
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

