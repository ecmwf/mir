// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>

#include "interpolate.h"


namespace mir::action::tests {


// the action is registered, but its output representation (and comparison) is not implemented


CASE("interpolate.grid2reduced-ll-pl-given, properties") {
    auto a = make("interpolate.grid2reduced-ll-pl-given");

    EXPECT(a->isRegridAction());
    EXPECT(!a->canCrop());

    std::string name;

    EXPECT(!a->getGriddedTargetName(name));

    EXPECT_EQUAL(a->type(), "interpolate.grid2reduced-ll-pl-given");
    EXPECT_EQUAL(std::string(a->name()), "Gridded2ReducedLLPLGiven");
    EXPECT(a->json_str().find(R"({"type":"interpolate.grid2reduced-ll-pl-given","interpolation":"linear")") == 0);

    std::ostringstream print;
    print << *a;

    EXPECT(print.str().find("Gridded2ReducedLLPLGiven[interpolation=linear") == 0);
}


CASE("interpolate.grid2reduced-ll-pl-given, errors") {
    EXPECT_THROWS(make("{type: interpolate.grid2reduced-ll-pl-given, interpolation: unknown}"));

    auto a = make("interpolate.grid2reduced-ll-pl-given");

    EXPECT_THROWS(a->outputBoundingBox());
    EXPECT_THROWS(perform(*a, field("{grid: O16}")));

    auto b = make("interpolate.grid2reduced-ll-pl-given");

    EXPECT_THROWS(static_cast<void>(*a == *b));
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
