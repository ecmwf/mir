// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/ArtificialInput.h"
#include "mir/input/GridSpecInput.h"
#include "mir/util/Exceptions.h"

#include "input.h"


namespace mir::api::tests {


CASE("ArtificialInput") {
    const std::string regular_ll =
        "gridded: true, gridType: regular_ll, north: 10, west: 0, south: 0, east: 10, west_east_increment: 1, "
        "south_north_increment: 1, Ni: 11, Nj: 11";


    SECTION("constant") {
        auto in = build("/dev/null", "{artificialInput: constant, constant: 42, " + regular_ll + "}");
        EXPECT(in->next());
        EXPECT_NOT(in->next());
        EXPECT_EQUAL(in->dimensions(), 1);

        auto field = in->field();
        EXPECT_EQUAL(field.values(0).size(), 121);
        for (const auto& v : field.values(0)) {
            EXPECT_EQUAL(v, 42.);
        }

        // additional keys (grid, paramId and numberOfDataPoints)
        const auto& param = in->parametrisation();

        std::vector<double> grid;
        EXPECT(param.get("grid", grid) && grid == std::vector<double>({1., 1.}));

        long paramId = 0;
        EXPECT(param.get("paramId", paramId) && paramId == 255);

        // the same input
        auto same    = build("/dev/null", "{artificialInput: constant, constant: 42, " + regular_ll + "}");
        auto another = build("/dev/null", "{artificialInput: constant, constant: 1, " + regular_ll + "}");
        EXPECT(in->sameAs(*same));
        EXPECT_NOT(in->sameAs(*another));
        EXPECT_NOT(in->sameAs(*build("regular_ll.2-2.grib1")));

        EXPECT(to_string(*in).find("ArtificialInput[parametrisation={") == 0);
    }


    SECTION("constant, rotated") {
        auto in = build("/dev/null",
                        "{artificialInput: constant, constant: 1, gridded: true, gridType: rotated_ll, north: 10, "
                        "west: 0, south: 0, east: 10, west_east_increment: 1, south_north_increment: 1, Ni: 11, "
                        "Nj: 11, south_pole_latitude: -40, south_pole_longitude: 22, south_pole_rotation_angle: 0}");
        EXPECT(in->next());

        std::vector<double> rotation;
        EXPECT(in->parametrisation().get("rotation", rotation));
        EXPECT(rotation == std::vector<double>({-40., 22.}));

        EXPECT_EQUAL(in->field().values(0).size(), 121);
    }


    SECTION("distribution") {
        auto in = build("/dev/null",
                        R"({artificialInput: distribution, distribution: "uniform-int-distribution{a: 1, b: 3}", )" +
                            regular_ll + "}");
        EXPECT(in->next());

        auto field         = in->field();
        const auto& values = field.values(0);
        EXPECT_EQUAL(values.size(), 121);
        for (const auto& v : values) {
            EXPECT(v == 1. || v == 2. || v == 3.);
        }
    }


    SECTION("dummy") {
        auto in = build("/dev/null", "{artificialInput: dummy}");
        EXPECT(in->next());

        auto field         = in->field();
        const auto& values = field.values(0);
        EXPECT_EQUAL(values.size(), 360 * 181);
        EXPECT(std::abs(values[0] - 1.) < 1e-12);  // sin(0) + cos(0)
    }


    SECTION("gridspec") {
        auto in = build("/dev/null", "{artificialInput: gridspec, gridSpec: {grid: O16}}");
        EXPECT(in->next());

        auto field = in->field();
        EXPECT_EQUAL(field.values(0).size(), 1600);
        EXPECT(std::all_of(field.values(0).begin(), field.values(0).end(), [](double v) { return v == 0.; }));

        std::string gridType;
        EXPECT(in->parametrisation().get("gridType", gridType) && gridType == "reduced_gg");

        // the key as before (gridspec), the same input; none, an error
        auto before = build("/dev/null", "{artificialInput: gridspec, gridspec: {grid: O16}}");
        EXPECT(before->next());
        EXPECT_EQUAL(before->field().values(0).size(), 1600);

        EXPECT_THROWS_AS(build("/dev/null", "{artificialInput: gridspec}"), exception::UserError);

        // spectral
        input_t sh(new input::GridSpecInput("{grid: T21}", false));
        EXPECT(sh->next());

        bool spectral = false;
        EXPECT(sh->parametrisation().get("spectral", spectral) && spectral);
        auto coefficients = sh->field();
        EXPECT_EQUAL(coefficients.values(0).size(), number_of_values(coefficients));
    }


    SECTION("unstructured grid, by griddef") {
        auto in = build("/dev/null",
                        "{artificialInput: constant, constant: 1, gridded: true, gridType: unstructured_grid, griddef: "
                        "tour.pts}");
        EXPECT(in->next());

        long n = 0;
        EXPECT(in->parametrisation().get("numberOfDataPoints", n) && n == 34);

        std::vector<double> latitudes;
        EXPECT(in->parametrisation().get("latitudes", latitudes) && latitudes.size() == 34);

        EXPECT_EQUAL(in->field().values(0).size(), 34);
    }


    SECTION("unstructured grid, by latitudes and longitudes files") {
        auto in =
            build("/dev/null",
                  "{artificialInput: constant, constant: 1, gridded: true, gridType: unstructured_grid, latitudes: "
                  "ICON.shortName=CLAT.grib2, longitudes: ICON.shortName=CLON.grib2}");
        EXPECT(in->next());

        std::vector<double> latitudes;
        EXPECT(in->parametrisation().get("latitudes", latitudes) && latitudes.size() == 20480);

        EXPECT_EQUAL(in->field().values(0).size(), 20480);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
