// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/RLE.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/GridSpecInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/repres/gauss/Gaussian.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"

#include "repres.h"


namespace mir::repres::tests {


using util::BoundingBox;


CASE("Reduced Gaussian grids: pl of the atlas named grids") {
    const std::vector<std::string> names{
        "N16",  "N24",  "N32",  "N48",  "N64",   "N80",   "N96",   "N128",  "N160",  "N200",  "N256",  "N320",
        "N400", "N512", "N576", "N640", "N800",  "N1024", "N1280", "N1600", "N2000", "N4000", "N8000", "O16",
        "O24",  "O32",  "O48",  "O64",  "O80",   "O96",   "O128",  "O160",  "O200",  "O256",  "O320",  "O400",
        "O512", "O576", "O640", "O800", "O1024", "O1280", "O1600", "O2000", "O4000", "O8000"};

    for (const auto& name : names) {
        atlas::ReducedGaussianGrid grid(name);
        const auto& pl = grid.nx();

        EXPECT_EQUAL(pl.size(), 2 * std::stoul(name.substr(1)));

        // symmetric, increasing towards the equator
        const auto half = pl.size() / 2;
        EXPECT(std::equal(pl.begin(), pl.begin() + static_cast<long>(half), pl.rbegin()));
        EXPECT(std::is_sorted(pl.begin(), pl.begin() + static_cast<long>(half)));

        std::vector<int> diff;
        eckit::DIFFencode(pl.begin(), pl.begin() + static_cast<long>(half), std::back_inserter(diff));

        std::vector<int> rle;
        eckit::RLEencode2(diff.begin(), diff.end(), std::back_inserter(rle), 1000);
        EXPECT(!rle.empty());

        if (name.front() == 'O') {
            // octahedral: 20 points at the poles, 4 more per latitude
            EXPECT_EQUAL(pl.front(), 20);
            EXPECT(std::all_of(diff.begin() + 1, diff.end(), [](int d) { return d == 4; }));
        }
    }
}


CASE("Reduced Gaussian grid") {
    using gauss::reduced::ReducedFromPL;

    const std::vector<long> pl{20, 24, 24, 20};
    RepresentationHandle global(new ReducedFromPL(2, pl, BoundingBox{}));
    const auto& lats = Gaussian::latitudes(2);

    EXPECT_EQUAL(global->numberOfPoints(), 88);
    EXPECT(global->isGlobal());
    EXPECT_EQUAL(global->factory(), "reduced_gg");
    EXPECT(global->uniqueName().find("R2-") == 0);


    SECTION("fillGrib") {
        grib_info info;
        global->fillGrib(info);

        EXPECT_EQUAL(info.grid.grid_type, CODES_UTIL_GRID_SPEC_REDUCED_GG);
        EXPECT_EQUAL(info.grid.N, 2);
        EXPECT_EQUAL(info.grid.Nj, 4);
        EXPECT_EQUAL(info.grid.pl_size, 4);
        EXPECT(std::equal(pl.begin(), pl.end(), info.grid.pl));
    }


    SECTION("cropped") {
        // (the area is not corrected to the grid points)
        RepresentationHandle cropped(global->croppedRepresentation({30, 5, -30, 85}));
        EXPECT_EQUAL(cropped->numberOfPoints(), 2 * 5);
        EXPECT_NOT(cropped->isGlobal());
        EXPECT_EQUAL(cropped->gridBoxes().size(), 2 * 5);

        grib_info info;
        cropped->fillGrib(info);
        EXPECT_EQUAL(info.grid.Nj, 2);
        EXPECT_EQUAL(info.grid.pl_size, 2);
        EXPECT_EQUAL(info.grid.pl[0], 24);
    }


    SECTION("extendBoundingBox (outwards, to the grid points)") {
        const BoundingBox bbox(10, 30, -10, 50);
        const auto extended = global->extendBoundingBox(bbox);

        EXPECT(extended.contains(bbox));
        EXPECT(extended == BoundingBox(lats[1], 0, lats[2], 60));

        const auto wide = global->extendBoundingBox({10, 1, -10, 350});
        EXPECT(eckit::types::is_approximately_equal(wide.west(), 0., PointLonLat::EPS));
        EXPECT(eckit::types::is_approximately_equal(wide.east(), LONGITUDE_GLOBE, PointLonLat::EPS));
    }


    SECTION("frame") {
        MIRValuesVector values(88, 1.);
        EXPECT_EQUAL(global->frame(values, 1, 9999.), 2 * 22);
        EXPECT_EQUAL(std::count(values.begin(), values.end(), 9999.), 2 * 22);
    }


    SECTION("element diagonal") {
        double d = 0.;
        EXPECT(global->getLongestElementDiagonal(d));
        EXPECT(d > 0.);
    }
}


CASE("Reduced Gaussian grid (by pl), rotated") {
    RepresentationHandle rotated(make("{pl: [20, 24, 24, 20], rotation: [-40, 22]}"));
    EXPECT_EQUAL(rotated->numberOfPoints(), 88);

    RepresentationHandle cropped(rotated->croppedRepresentation({30, -30, -30, 30}));
    EXPECT(cropped->numberOfPoints() < 88);
    EXPECT_EQUAL(count(*cropped), cropped->numberOfPoints());
    EXPECT_NOT(cropped->sameAs(*rotated));
    EXPECT_NOT(cropped->geographicDomain() == cropped->domain());
}


CASE("Reduced octahedral Gaussian grid: cropped across the date line (PGEN-558)") {
    RepresentationHandle repres(new gauss::reduced::ReducedOctahedral(64));
    RepresentationHandle cropped(repres->croppedRepresentation({73, 80, 70, 280}));

    const auto& crop = cropped->boundingBox();
    EXPECT(eckit::types::is_approximately_equal(crop.west(), 80., PointLonLat::EPS));
    EXPECT(eckit::types::is_approximately_equal(crop.east(), 280., PointLonLat::EPS));

    // interpolation to the same area, from a global or a regular Gaussian grid (validating the representation)
    api::MIRJob job;
    job.set("grid", "O64");
    job.set("interpolation", "nn");
    job.set("dont-compress-plan", true);
    job.set("area", std::vector<double>{73, 80, 70, 280});

    const std::vector<std::string> inputs{"{grid: O64}", "{grid: F64}"};
    for (const auto& gridspec : inputs) {
        SECTION(gridspec) {
            output::EmptyOutput output;
            std::unique_ptr<input::MIRInput> input(new input::GridSpecInput(gridspec, true));
            EXPECT_NO_THROW(job.execute(*input, output));
        }
    }
}


CASE("Reduced Gaussian grids: globalise (to the global grid, the values in place)") {
    // the pl of O8, as given
    std::string pl;
    for (long j = 0; j < 16; ++j) {
        pl += (j == 0 ? "" : ", ") + std::to_string(20 + 4 * (j < 8 ? j : 15 - j));
    }

    struct test_t {
        std::string grid;
        std::string area;
    };

    for (const auto& test : std::vector<test_t>{
             {"grid: N16", "area: [60, -10, 30, 40]"},
             {"grid: O16", "area: [60, -10, 30, 40]"},
             {"pl: [" + pl + "]", "area: [60, -10, 30, 40]"},
             {"grid: O16, rotation: [-40, 22]", "area: [20, -20, -20, 20]"},
             {"grid: N16, rotation: [-40, 22]", "area: [20, 350, -20, 380]"},
         }) {
        SECTION(test.grid + ", " + test.area) {
            RepresentationHandle regional(make("{" + test.grid + ", " + test.area + "}"));
            RepresentationHandle expected(make("{" + test.grid + "}"));
            EXPECT_NOT(regional->isGlobal());

            // values by point
            auto value = [](const PointLonLat& p) { return p.lat() * 1000. + util::normalise_longitude(p.lon(), 0.); };

            const auto n = regional->numberOfPoints();
            MIRValuesVector values(n);
            const auto& points = regional->canonicalGridUnrotated();
            for (auto it = points.cbegin(); it; ++it) {
                values.at(it.index()) = value(std::get<PointLonLat>(*it));
            }

            data::MIRField field(regional, false, 9999.);
            field.update(values, 0);

            RepresentationHandle global(regional->globalise(field));
            ASSERT(global);

            EXPECT(global->isGlobal());
            EXPECT(global->sameAs(*expected));
            EXPECT(field.hasMissing());

            // the values in place, missing values elsewhere
            const auto& v = field.values(0);
            EXPECT_EQUAL(v.size(), expected->numberOfPoints());

            size_t count             = 0;
            const auto& globalPoints = global->canonicalGridUnrotated();
            for (auto it = globalPoints.cbegin(); it; ++it) {
                if (const auto x = v.at(it.index()); x != 9999.) {
                    EXPECT(same(x, value(std::get<PointLonLat>(*it)), 1e-6));
                    ++count;
                }
            }
            EXPECT_EQUAL(count, n);

            // already global
            data::MIRField again(global, true, 9999.);
            again.update(field.direct(0), 0);
            EXPECT(global->globalise(again) == nullptr);
        }
    }
}


}  // namespace mir::repres::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
