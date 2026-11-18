// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "action.h"


namespace mir::action::tests {


CASE("set.metadata") {
    auto out = perform("{type: set.metadata, metadata: 'paramId=167,level=2'}", field("{grid: [10, 10]}", F{}, 2));

    for (size_t which = 0; which < out.dimensions(); ++which) {
        const auto& md = out.metadata(which);

        EXPECT_EQUAL(md.at("paramId"), 167);
        EXPECT_EQUAL(md.at("level"), 2);
    }
}


CASE("set.metadata, comparison and errors") {
    EXPECT(*make("{type: set.metadata, metadata: paramId=1}") != *make("{type: set.metadata, metadata: paramId=2}"));
    EXPECT_THROWS(make("{type: set.metadata, metadata: paramId}"));
}


CASE("set.metadata, printing") {
    EXPECT(str(*make("{type: set.metadata, metadata: paramId=1}")).rfind("SetMetadata[", 0) == 0);
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
