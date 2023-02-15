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


#include "eckit/testing/Test.h"

#include "mir/key/Area.h"
#include "mir/util/BoundingBox.h"


namespace mir::tests::unit {


CASE("Area") {
    util::BoundingBox europe;
    EXPECT(key::Area::match("europe", europe));

    util::BoundingBox france;
    EXPECT(key::Area::match("france", france));

    util::BoundingBox uk;
    EXPECT(key::Area::match("uk", uk));

    util::BoundingBox global;
    EXPECT(key::Area::match("global", global));

    EXPECT(europe.contains(france));
    EXPECT_NOT(france.contains(europe));

    EXPECT(europe.contains(uk));
    EXPECT_NOT(uk.contains(europe));

    EXPECT_NOT(france.contains(uk));
    EXPECT_NOT(uk.contains(france));

    EXPECT(global.contains(europe));
    EXPECT_NOT(europe.contains(global));
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
