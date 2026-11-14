// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>

#include "interpolate.h"


namespace mir::tests::actions {


// the action is registered, but its output representation (and comparison) is not implemented


CASE("interpolate.grid2reduced-ll, properties") {
    auto a = make("interpolate.grid2reduced-ll");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-ll");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedLL");
    EXPECT(a->json_str().find(R"({"type":"interpolate.grid2reduced-ll","interpolation":"linear")") == 0);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedLL[interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-ll, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2reduced-ll, interpolation: unknown}"));

    auto a = make("interpolate.grid2reduced-ll");

    EXPECT_THROWS(a->outputBoundingBox());
    EXPECT_THROWS(perform(*a, field("{grid: O16}")));

    auto b = make("interpolate.grid2reduced-ll");

    EXPECT_THROWS(static_cast<void>(*a == *b));
}


}  // namespace mir::tests::actions


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
