// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/param/SpecParametrisation.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("SpecParametrisation") {
    SECTION("parse") {
        EXPECT(param::SpecParametrisation::parse("", "key").empty());
        EXPECT(param::SpecParametrisation::parse("  ", "key").empty());

        SimpleParametrisation name;
        param::SpecParametrisation::parse(" linear ", "interpolation").set(name);
        EXPECT_EQUAL(to_string(name), R"({"interpolation":"linear"})");

        SimpleParametrisation spec;
        param::SpecParametrisation::parse("{type: k-nearest, nclosest: 8, distance_tolerance: 2}", "interpolation")
            .set(spec);
        EXPECT_EQUAL(to_string(spec), R"({"distance-tolerance":2,"interpolation":"k-nearest","nclosest":8})");

        // nested specs (as JSON strings), the values as they are
        SimpleParametrisation nested;
        param::SpecParametrisation::parse("{type: x, grid: {latitudes: [37.742114, 0.1]}}", "key").set(nested);

        std::string grid;
        EXPECT(nested.get("grid", grid));

        SimpleParametrisation values;
        param::SpecParametrisation::parse(grid, "key").set(values);

        std::vector<double> latitudes;
        EXPECT(values.get("latitudes", latitudes) && latitudes == std::vector<double>({37.742114, 0.1}));
    }


    SECTION("as user parametrisation, combined with the defaults") {
        const param::SpecParametrisation spec("{type: k-nearest, nclosest: 8}", "interpolation");
        const MIRParametrisation& p = spec;

        std::string interpolation;
        EXPECT(p.get("interpolation", interpolation) && interpolation == "k-nearest");

        long nclosest = 0;
        EXPECT(p.get("nclosest", nclosest) && nclosest == 8);

        double distance = 0;
        EXPECT(p.get("distance", distance) && distance == 1.);

        EXPECT(p.userParametrisation().has("nclosest"));
        EXPECT_NOT(p.userParametrisation().has("distance"));
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
