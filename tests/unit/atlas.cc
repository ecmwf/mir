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


#include <memory>
#include <ostream>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"

#include "mir/key/grid/Grid.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Domain.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::tests::unit {


using Handle = repres::RepresentationHandle;


CASE("Test number of points representation <=> grid") {
    auto& log = Log::info();

    using key::grid::Grid;
    using repres::latlon::RegularLL;


    Handle representations[] = {
//        new RegularLL(util::Increments(1., 1.)),
//        new RegularLL(util::Increments(1., 1.), util::BoundingBox()),
//        Grid::lookup("O16").representation(),
//        Grid::lookup("O1280").representation(),
        Grid::lookup("H2").representation(),
    };

    for (const auto& repres : representations) {
        size_t n1 = repres->numberOfPoints();
        log << "#=" << n1 << "\tfrom " << *repres << std::endl;

        auto grid = repres->atlasGrid();
        auto n2   = size_t(grid.size());
        log << "#=" << n2 << "\tfrom " << grid.spec() << std::endl;

        EXPECT(n1 == n2);
    }
}


#if 0
CASE("Test number of points representation <=> cropped grid") {
    auto& log = Log::info();

    using repres::latlon::RegularLL;

    const util::Domain domains[] = {
        {90, 0, 90, 360},       // North pole
        {90, 0, 89, 360},       //
        {0, 0, 0, 360},         // equator
        {-89, 0, -90, 360},     //
        {-90, 0, -90, 360},     // South pole
        {90, 0, -90, 0},        // Greenwhich
        {90, 0, -90, 1},        //
        {90, 180, -90, 180},    // date line
        {90, -180, -90, -180},  // date line
        {90, -1, -90, 0},       //
        {90, -1, -90, -1},      //
    };

    for (const auto& dom : domains) {
        ASSERT(!dom.isGlobal());

        Handle repres(new RegularLL(util::Increments(1., 1.)));
        Handle represCropped(repres->croppedRepresentation(dom));

        size_t n1 = represCropped->numberOfPoints();
        log << "#=" << n1 << "\tfrom " << *represCropped << std::endl;

        auto grid = repres->atlasGrid();
        ASSERT(grid.domain().global());

        auto gridCropped = atlas::Grid(grid, dom);
        ASSERT(!gridCropped.domain().global());

        auto n2 = size_t(gridCropped.size());
        log << "#=" << n2 << "\tfrom " << gridCropped.spec() << std::endl;

        EXPECT(n1 == n2);
    }
}


CASE("MIR-374") {
    auto& log  = Log::info();
    auto old   = log.precision(16);
    double eps = 1.e-6;

    const util::Domain domains[] = {
        {90, -180, -90, 180},
        {90, -180, -60, 180},
        {10, 0, 0, 10},
    };

    const std::string names[] = {"O16", "O640"};

    for (const auto& domain : domains) {
        for (const auto& name : names) {
            Handle repr = key::grid::Grid::lookup(name).representation();
            Handle crop = repr->croppedRepresentation(domain);

            std::unique_ptr<repres::Iterator> it(crop->iterator());
            ASSERT(it->next());
            PointLatLon p = it->pointUnrotated();
            log << p << std::endl;

            auto grid            = crop->atlasGrid();
            atlas::PointLonLat q = *(grid.lonlat().begin());

            log << "Compare p=" << p << " with q=" << q << std::endl;
            EXPECT(eckit::types::is_approximately_equal(p.lat().value(), q.lat(), eps));
            EXPECT(eckit::types::is_approximately_equal(p.lon().value(), q.lon(), eps));
        }
    }

    log.precision(old);
}
#endif


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
