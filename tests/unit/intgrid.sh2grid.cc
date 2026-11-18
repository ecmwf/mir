// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/input/GribFileInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "intgrid.h"


namespace mir::tests::unit {


CASE("sh2grid: style option sh2grid-intgrid") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    SECTION("regular-gg-from-target (default, unset or empty)") {
        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());

        for (const std::string value : {"regular-gg-from-target", ""}) {
            Plan direct(user.set("sh2grid-intgrid", value), field);
            EXPECT(direct.has("ShToNamedGrid", {"grid=O320"}));
            EXPECT(!direct.interpolates());
        }
    }

    SECTION("compatible") {
        Plan plan(user.set("sh2grid-intgrid", "compatible"), field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=O320"}));
    }

    SECTION("compatible, with intgrid set") {
        Plan plan(user.set("sh2grid-intgrid", "compatible").set("intgrid", "regular-gg-from-target"), field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("invalid") {
        user.set("sh2grid-intgrid", "?");
        EXPECT_THROWS_AS(Plan plan(user, field), exception::UserError);
    }

    SECTION("GRIB input") {
        std::unique_ptr<input::MIRInput> input(new input::GribFileInput("mtg2-t.grib2"));
        ASSERT(input->next());

        const auto& grib = input->parametrisation();

        Plan plan(user, grib);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());

        Plan compatible(user.set("sh2grid-intgrid", "compatible"), grib);
        EXPECT(compatible.has("ShToNamedGrid", {"grid=F"}));
        EXPECT(compatible.has("Gridded2NamedGrid", {"grid=O320"}));
    }
}


CASE("sh2grid: spectral to Gaussian grids, directly (default)") {
    const SpectralField field(1279);

    SECTION("octahedral, named") {
        param::SimpleParametrisation user;
        user.set("grid", "O320");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"invtrans=<scalar>", "grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("octahedral, named (lowercase)") {
        param::SimpleParametrisation user;
        user.set("grid", "o320");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("reduced classic, named") {
        param::SimpleParametrisation user;
        user.set("grid", "N320");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=N320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("regular, named") {
        param::SimpleParametrisation user;
        user.set("grid", "F320");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("octahedral, by Gaussian number") {
        param::SimpleParametrisation user;
        user.set("octahedral", 320L);

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToOctahedralGG"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("reduced classic, by Gaussian number") {
        param::SimpleParametrisation user;
        user.set("reduced", 320L);

        Plan plan(user, field);
        EXPECT(plan.has("ShToReducedGG"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("regular, by Gaussian number") {
        param::SimpleParametrisation user;
        user.set("regular", 320L);

        Plan plan(user, field);
        EXPECT(plan.has("ShToRegularGG"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("reduced, by pl") {
        param::SimpleParametrisation user;
        user.set("pl", std::vector<long>{20, 24, 24, 20});

        Plan plan(user, field);
        EXPECT(plan.has("ShToReducedGGPLGiven"));
        EXPECT(!plan.has("ShToNamedGrid"));
        EXPECT(!plan.interpolates());
    }

    SECTION("gridspec") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320}");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToGridSpec", {"cropping=none", R"(gridspec={"grid":"O320"})"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("gridspec, regional (global grid, cropped)") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320, area: [60, -10, 30, 40]}");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToGridSpec", {"cropping=BoundingBox[", R"(gridspec={"grid":"O320"})"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("source") {
        param::SimpleParametrisation user;
        user.set("grid", "source");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O1280"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("target grid finer than input truncation") {
        param::SimpleParametrisation user;
        user.set("grid", "O1280");

        Plan plan(user, SpectralField(639));
        EXPECT(!plan.has("ShTruncate"));
        EXPECT(plan.has("ShToNamedGrid", {"grid=O1280"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("area") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("area", std::vector<double>{60, -10, 30, 40});

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(plan.has("AreaCropper"));
        EXPECT(!plan.interpolates());
    }

    SECTION("vod2uv") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("vod2uv", true);

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"invtrans=<vod2uv>", "grid=O320"}));
        EXPECT(!plan.has("AdjustWindsScaleCosLatitude"));
        EXPECT(!plan.interpolates());
    }

    SECTION("U/V input") {
        param::SimpleParametrisation user;
        user.set("grid", "O320");

        Plan plan(user, SpectralField(1279, true));
        auto transform = plan.find("ShToNamedGrid", {"invtrans=<scalar>", "grid=O320"});
        auto scale     = plan.find("AdjustWindsScaleCosLatitude");

        EXPECT(transform != Plan::npos);
        EXPECT(scale != Plan::npos);
        EXPECT(transform < scale);
        EXPECT(!plan.interpolates());
    }

    SECTION("intgrid=none") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("intgrid", "none");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }
}


CASE("sh2grid: spectral to Gaussian grids, with intermediate grid") {
    const SpectralField field(1279);

    SECTION("intgrid=O640") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("intgrid", "O640");

        Plan plan(user, field);
        auto transform   = plan.find("ShToNamedGrid", {"grid=O640"});
        auto interpolate = plan.find("Gridded2NamedGrid", {"grid=O320"});

        EXPECT(transform != Plan::npos);
        EXPECT(interpolate != Plan::npos);
        EXPECT(transform < interpolate);
    }

    SECTION("intgrid=O320 (the target grid)") {
        param::SimpleParametrisation user;
        user.set("grid", "o320").set("intgrid", "O320");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("intgrid=source") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("intgrid", "source");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O1280"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=O320"}));
    }

    SECTION("rotated") {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("rotation", std::vector<double>{-40, 22});

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2RotatedNamedGrid", {"grid=O320"}));
    }

    SECTION("gridspec, rotated") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320, projection: {type: rotation, south_pole: [-40, 22]}}");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2GridSpec"));
    }

    SECTION("intgrid=regular-gg-from-target-compatible (and aliases)") {
        struct test_t {
            std::string key;
            std::string value;
            std::string interpolate;
        };

        for (const std::string intgrid : {"regular-gg-from-target-compatible", "automatic", "auto", "AUTO"}) {
            for (const auto& test : std::vector<test_t>{
                     {"grid", "O320", "Gridded2NamedGrid"},
                     {"grid", "N320", "Gridded2NamedGrid"},
                     {"octahedral", "320", "Gridded2OctahedralGG"},
                     {"reduced", "320", "Gridded2ReducedGG"},
                     {"regular", "320", "Gridded2RegularGG"},
                 }) {
                param::SimpleParametrisation user;
                user.set("intgrid", intgrid);
                if (test.key == "grid") {
                    user.set(test.key, test.value);
                }
                else {
                    user.set(test.key, std::stol(test.value));
                }

                Plan plan(user, field);
                auto transform   = plan.find("ShToNamedGrid", {"grid=F320"});
                auto interpolate = plan.find(test.interpolate);

                EXPECT(plan.has("ShTruncate", {"truncation=639"}));
                EXPECT(transform != Plan::npos);
                EXPECT(interpolate != Plan::npos);
                EXPECT(transform < interpolate);
            }
        }
    }

    SECTION("intgrid=regular-gg-from-target-compatible, gridspec") {
        param::SimpleParametrisation user;
        user.set("grid", "{grid: O320}").set("intgrid", "regular-gg-from-target-compatible");

        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(plan.has("Gridded2GridSpec"));
    }

    SECTION("intgrid=regular-gg-from-target-compatible, F320 (the target grid)") {
        param::SimpleParametrisation user;
        user.set("grid", "F320").set("intgrid", "regular-gg-from-target-compatible");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F320"}));
        EXPECT(!plan.interpolates());
    }

    SECTION("intgrid=regular-gg-from-target-compatible, target grid finer than input truncation") {
        param::SimpleParametrisation user;
        user.set("grid", "O1280").set("intgrid", "regular-gg-from-target-compatible");

        Plan plan(user, SpectralField(639));
        EXPECT(plan.has("ShToNamedGrid", {"grid=F640"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=O1280"}));
    }
}


CASE("sh2grid: spectral to non-Gaussian grids, with intermediate grid") {
    const SpectralField field(1279);

    SECTION("regular lat/lon") {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1, 1});

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F90"}));
        EXPECT(plan.has("Gridded2RegularLL"));
    }

    SECTION("HEALPix") {
        param::SimpleParametrisation user;
        user.set("grid", "H32");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=F64"}));
        EXPECT(plan.has("Gridded2NamedGrid", {"grid=H32"}));
    }
}


CASE("sh2grid: spectral truncation, from the inverse transform grid") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    SECTION("target grid") {
        Plan plan(user, field);
        EXPECT(plan.has("ShTruncate", {"truncation=639"}));
    }

    SECTION("intermediate grid") {
        Plan coarser(user.set("intgrid", "F160"), field);
        EXPECT(coarser.has("ShTruncate", {"truncation=319"}));
        EXPECT(coarser.has("ShToNamedGrid", {"grid=F160"}));
        EXPECT(coarser.has("Gridded2NamedGrid", {"grid=O320"}));

        Plan finer(user.set("intgrid", "O640"), field);
        EXPECT(!finer.has("ShTruncate"));
        EXPECT(finer.has("ShToNamedGrid", {"grid=O640"}));
    }

    SECTION("spectral-order=cubic") {
        Plan plan(user.set("spectral-order", "cubic"), field);
        EXPECT(plan.has("ShTruncate", {"truncation=319"}));
    }

    SECTION("truncation=213") {
        Plan plan(user.set("truncation", "213"), field);
        EXPECT(plan.has("ShTruncate", {"truncation=213"}));
    }

    SECTION("truncation=none") {
        Plan plan(user.set("truncation", "none"), field);
        EXPECT(!plan.has("ShTruncate"));
    }

    SECTION("spectral output") {
        param::SimpleParametrisation spectral;

        Plan plan(spectral, field);
        EXPECT(!plan.has("ShTruncate"));

        Plan truncated(spectral.set("truncation", "639"), field);
        EXPECT(truncated.has("ShTruncate", {"truncation=639"}));
        EXPECT(!truncated.has("ShToNamedGrid"));
    }
}


CASE("sh2grid: U/V input, wind scaling by cos(latitude) and intermediate grid") {
    const SpectralField field(1279, true);

    // empty option is the default
    for (const std::string wind : {"default", "compatible", ""}) {
        param::SimpleParametrisation user;
        user.set("grid", std::vector<double>{1, 1}).set("sh2grid-wind", wind);

        Plan plan(user, field);
        auto transform   = plan.find("ShToNamedGrid");
        auto scale       = plan.find("AdjustWindsScaleCosLatitude");
        auto interpolate = plan.find("Gridded2RegularLL");

        EXPECT(transform != Plan::npos);
        EXPECT(scale != Plan::npos);
        EXPECT(interpolate != Plan::npos);
        EXPECT(transform < scale && transform < interpolate);

        // wind scaling on the (Gaussian) intermediate grid, compatible mode on the target grid
        EXPECT(wind == "compatible" ? interpolate < scale : scale < interpolate);
        EXPECT(!plan.has("AdjustWindsDirections"));
    }
}


CASE("sh2grid: U/V input, rotated") {
    const SpectralField field(1279, true);

    for (const std::string wind : {"default", "compatible", ""}) {
        param::SimpleParametrisation user;
        user.set("grid", "O320").set("rotation", std::vector<double>{-40, 22}).set("sh2grid-wind", wind);

        Plan plan(user, field);
        auto scale      = plan.find("AdjustWindsScaleCosLatitude");
        auto directions = plan.find("AdjustWindsDirections");

        EXPECT(scale != Plan::npos);
        EXPECT(directions != Plan::npos);
        EXPECT(scale < directions);
    }
}


CASE("sh2grid: incompatible options") {
    param::SimpleParametrisation user;
    user.set("grid", "O320").set("vod2uv", true);

    SECTION("vod2uv and U/V input") {
        EXPECT_THROWS_AS(Plan plan(user, SpectralField(1279, true)), exception::UserError);
    }

    SECTION("vod2uv and uv2uv") {
        user.set("uv2uv", true);
        EXPECT_THROWS_AS(Plan plan(user, SpectralField(1279)), exception::UserError);
    }
}


CASE("sh2grid: style option sh2grid-wind") {
    const SpectralField field(1279);

    param::SimpleParametrisation user;
    user.set("grid", "O320");

    SECTION("invalid") {
        user.set("sh2grid-wind", "?");
        EXPECT_THROWS_AS(Plan plan(user, field), exception::UserError);
    }

    SECTION("empty is the default") {
        user.set("sh2grid-wind", "");

        Plan plan(user, field);
        EXPECT(plan.has("ShToNamedGrid", {"grid=O320"}));
        EXPECT(!plan.interpolates());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
