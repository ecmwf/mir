// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/param/GridSpecParametrisation.h"
#include "mir/util/Exceptions.h"

#include "parametrisation.h"


namespace mir::api::tests {


CASE("GridSpecParametrisation") {
    struct test_t {
        std::string gridspec;
        std::string gridType;
        std::string grid;
    };


    SECTION("gridType and grid") {
        for (const auto& test : std::vector<test_t>{
                 {"{grid: F16}", "regular_gg", "F16"},
                 {"{grid: N16}", "reduced_gg", "N16"},
                 {"{grid: O16}", "reduced_gg", "O16"},
                 {"{grid: O16, rotation: [-40, 22]}", "reduced_rotated_gg", "O16"},
                 {"{grid: F16, rotation: [-40, 22]}", "rotated_gg", "F16"},
                 {"{grid: H4}", "healpix", "H4"},
                 {"{grid: H4, order: nested}", "healpix", "HN4"},
                 {"{grid: eORCA1_T}", "unstructured_grid", "eORCA1_T"},
             }) {
            const param::GridSpecParametrisation p(test.gridspec);
            const MIRParametrisation& m = p;

            std::string gridType;
            EXPECT(m.get("gridType", gridType));
            EXPECT_EQUAL(gridType, test.gridType);

            std::string grid;
            EXPECT(m.get("grid", grid));
            EXPECT_EQUAL(grid, test.grid);

            bool gridded = false;
            EXPECT(m.get("gridded", gridded) && gridded);
            EXPECT_NOT(m.has("spectral"));
            EXPECT_NOT(m.has("gridded_regular_ll"));

            std::string gridSpec;
            EXPECT(m.get("gridSpec", gridSpec) && !gridSpec.empty());

            EXPECT_EQUAL(to_string(m), "GridSpecParametrisation[" + gridSpec + "]");
        }
    }


    SECTION("regular_ll") {
        const param::GridSpecParametrisation p("{grid: [2, 1], area: [10, 0, -10, 20]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "regular_ll");

        bool b = false;
        EXPECT(m.get("gridded_regular_ll", b) && b);

        std::vector<double> grid;
        EXPECT(m.get("grid", grid) && grid == std::vector<double>({2., 1.}));

        long Ni = 0;
        long Nj = 0;
        EXPECT(m.get("Ni", Ni) && Ni == 11);
        EXPECT(m.get("Nj", Nj) && Nj == 21);

        double north = 0;
        double south = 0;
        double west  = 0;
        double east  = 0;
        double we    = 0;
        double sn    = 0;
        EXPECT(m.get("north", north) && north == 10.);
        EXPECT(m.get("south", south) && south == -10.);
        EXPECT(m.get("west", west) && west == 0.);
        EXPECT(m.get("east", east) && east == 20.);
        EXPECT(m.get("west_east_increment", we) && we == 2.);
        EXPECT(m.get("south_north_increment", sn) && sn == 1.);

        std::string order;
        EXPECT(m.get("order", order) && !order.empty());

        EXPECT_NOT(m.has("gridName"));
        EXPECT_NOT(m.has("pl"));
        EXPECT_NOT(m.has("N"));
        EXPECT_NOT(m.has("rotation"));
        EXPECT_NOT(m.has("truncation"));
        EXPECT_NOT(m.has("uid"));
        EXPECT_NOT(m.has("latitudes/longitudes"));
        EXPECT_NOT(m.has("unknown"));

        // getters of other types
        int i = 0;
        EXPECT(m.get("Ni", i) && i == 11);

        float f = 0;
        EXPECT_NOT(m.get("unknown", f));

        std::vector<int> vi;
        EXPECT_NOT(m.get("unknown", vi));

        std::vector<long> vl;
        EXPECT_NOT(m.get("unknown", vl));

        std::vector<float> vf;
        EXPECT_NOT(m.get("unknown", vf));

        std::vector<std::string> vs;
        EXPECT_THROWS_AS(m.get("gridType", vs), eckit::NotImplemented);
    }


    SECTION("rotated regular_ll") {
        const param::GridSpecParametrisation p("{grid: [1, 1], area: [10, -10, -10, 10], rotation: [-40, 22]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "rotated_ll");

        double lat   = 0;
        double lon   = 0;
        double angle = 1;
        EXPECT(m.get("south_pole_latitude", lat) && lat == -40.);
        EXPECT(m.get("south_pole_longitude", lon) && lon == 22.);
        EXPECT(m.get("south_pole_rotation_angle", angle) && angle == 0.);

        std::vector<double> rotation;
        EXPECT(m.get("rotation", rotation) && rotation == std::vector<double>({-40., 22.}));
    }


    SECTION("regular_gg") {
        const param::GridSpecParametrisation p("{grid: F16}");
        const MIRParametrisation& m = p;

        long N = 0;
        EXPECT(m.get("N", N) && N == 16);
        EXPECT(m.get("regular", N) && N == 16);

        long Ni = 0;
        long Nj = 0;
        EXPECT(m.get("Ni", Ni) && Ni == 64);
        EXPECT(m.get("Nj", Nj) && Nj == 32);

        double we = 0;
        EXPECT(m.get("west_east_increment", we) && we == 360. / 64.);
        EXPECT_NOT(m.has("south_north_increment"));

        std::string gridName;
        EXPECT(m.get("gridName", gridName) && gridName == "F16");
    }


    SECTION("reduced_gg") {
        for (const auto& test :
             std::vector<test_t>{{"{grid: O16}", "octahedral", "O16"}, {"{grid: N16}", "reduced", "N16"}}) {
            const param::GridSpecParametrisation p(test.gridspec);
            const MIRParametrisation& m = p;

            long N = 0;
            EXPECT(m.get("N", N) && N == 16);
            EXPECT(m.get(test.gridType, N) && N == 16);

            std::vector<long> pl;
            EXPECT(m.get("pl", pl) && pl.size() == 32);

            long Nj = 0;
            EXPECT(m.get("Nj", Nj) && Nj == 32);

            double north = 0;
            double south = 0;
            double west  = 1;
            double east  = 0;
            EXPECT(m.get("north", north) && 0. < north && north < 90.);
            EXPECT(m.get("south", south) && same(south, -north));
            EXPECT(m.get("west", west) && west == 0.);
            EXPECT(m.get("east", east) && 350. < east && east < 360.);

            std::string order;
            EXPECT(m.get("order", order) && !order.empty());
        }


        // a regional grid keeps all latitudes, those without points have no longitudes
        const param::GridSpecParametrisation p("{grid: O16, area: [60, -10, 30, 40]}");
        const MIRParametrisation& m = p;

        double north = 0;
        double south = 0;
        double west  = 0;
        double east  = 0;
        long Nj      = 0;
        EXPECT(m.get("north", north) && north <= 60. && north > 50.);
        EXPECT(m.get("south", south) && south >= 30. && south < 40.);
        EXPECT(m.get("west", west) && west >= -10. && west < 0.);
        EXPECT(m.get("east", east) && east <= 40. && east > 30.);
        EXPECT(m.get("Nj", Nj) && 0 < Nj && Nj < 32);
    }


    SECTION("reduced_ll") {
        const param::GridSpecParametrisation p("{type: reduced_ll, pl: [4, 8, 8, 4]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "reduced_ll");

        std::vector<long> pl;
        EXPECT(m.get("pl", pl) && pl == std::vector<long>({4, 8, 8, 4}));
    }


    SECTION("HEALPix") {
        for (const auto& order : std::vector<std::string>{"ring", "nested"}) {
            const param::GridSpecParametrisation p("{grid: H4, order: " + order + "}");
            const MIRParametrisation& m = p;

            long Nside = 0;
            EXPECT(m.get("Nside", Nside) && Nside == 4);

            std::string ordering;
            EXPECT(m.get("orderingConvention", ordering) && ordering == order);

            std::string gridName;
            EXPECT(m.get("gridName", gridName) && gridName == "H4");

            double west = 0;
            EXPECT(m.get("west", west) && west == 45.);
            EXPECT(m.get("longitudeOfFirstGridPointInDegrees", west) && west == 45.);

            EXPECT_NOT(m.has("pl"));
        }
    }


    SECTION("spherical harmonics") {
        const param::GridSpecParametrisation p("{grid: T21}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "sh");

        bool spectral = false;
        EXPECT(m.get("spectral", spectral) && spectral);
        EXPECT_NOT(m.has("gridded"));
        EXPECT_NOT(m.has("grid"));

        long truncation = 0;
        EXPECT(m.get("truncation", truncation) && truncation == 21);
    }


    SECTION("unstructured") {
        const param::GridSpecParametrisation orca("{grid: eORCA1_T}");

        std::string uid;
        EXPECT(static_cast<const MIRParametrisation&>(orca).get("uid", uid) && uid.size() == 32);
        EXPECT_NOT(static_cast<const MIRParametrisation&>(orca).has("latitudes/longitudes"));

        const param::GridSpecParametrisation p(
            "{type: unstructured_ll, latitudes: [0, 10, 20], longitudes: [0, 10, 20]}");
        const MIRParametrisation& m = p;

        std::string gridType;
        EXPECT(m.get("gridType", gridType) && gridType == "unstructured_ll");
        EXPECT(m.has("latitudes/longitudes"));

        std::vector<double> lats;
        std::vector<double> lons;
        EXPECT(m.get("latitudes", lats) && lats == std::vector<double>({0., 10., 20.}));
        EXPECT(m.get("longitudes", lons) && lons == std::vector<double>({0., 10., 20.}));
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
