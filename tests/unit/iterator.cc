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


#include <ios>

#include "eckit/testing/Test.h"
#include "eckit/types/Fraction.h"

#include "mir/iterator/detail/RegularIterator.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Increments.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


using eckit::Fraction;
using util::BoundingBox;
using util::Increments;

static auto& log = Log::info();


#define EXPECTV(a)                                \
    log << "\tEXPECT(" << #a << ")" << std::endl; \
    EXPECT(a)


CASE("MIR-390") {

    auto old(log.precision(16));
    log << std::boolalpha;

    // 89.9958333333, 0.004166666667, -89.9958333333, 359.995833333
    BoundingBox bbox{Fraction(21599, 240), Fraction(1, 240), Fraction(-21599, 240), Fraction(86355, 240)};
    PointLatLon reference{bbox.south(), bbox.west()};

    size_t Ni = 43200;
    size_t Nj = 21600;

    Increments inc((bbox.east().fraction() - bbox.west().fraction()) / Ni,
                   (bbox.north().fraction() - bbox.south().fraction()) / Nj);

    SECTION("LatLon::correctBoundingBox") {

        using iterator::detail::RegularIterator;

        // Latitude/longitude ranges
        RegularIterator lat{bbox.south().fraction(), bbox.north().fraction(), inc.south_north().latitude().fraction(),
                            reference.lat().fraction()};

        auto n = lat.b();
        auto s = lat.a();

        EXPECTV(bbox.south().fraction() == s);
        EXPECTV(bbox.north().fraction() == n);
        EXPECTV(Nj = lat.n());

        RegularIterator lon{bbox.west().fraction(), bbox.east().fraction(), inc.west_east().longitude().fraction(),
                            reference.lon().fraction(), Longitude::GLOBE.fraction()};

        auto w = lon.a();
        auto e = lon.b();

        EXPECTV(bbox.west().fraction() == w);
        EXPECTV(bbox.east().fraction() == e);
        EXPECTV(Ni = lon.n());

        // checks
        ASSERT(w + (Ni - 1) * lon.inc() == e || Ni * lon.inc() == Longitude::GLOBE.fraction());
        ASSERT(s + (Nj - 1) * lat.inc() == n);

        EXPECTV(bbox == BoundingBox(n, w, s, e));
    }


    log.precision(old);
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
