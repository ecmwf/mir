// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/output/EmptyOutput.h"
#include "mir/output/GmshOutput.h"
#include "mir/output/PNGOutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "output.h"


namespace mir::api::tests {


CASE("MIROutputFactory") {
    eckit::TmpDir dir;

    auto build = [&dir](const std::string& name, const param::SimpleParametrisation& user) {
        const param::CombinedParametrisation param(user);
        return output_t(output::MIROutputFactory::build((dir / name).asString(), param));
    };

    struct test_t {
        std::string name;
        std::string format;
        std::string print;
    };


    SECTION("by format") {
        for (const auto& test : std::vector<test_t>{
                 {"a.grib", "grib", "GribFileOutput[path="},
                 {"a", "empty", "EmptyOutput[]"},
                 {"a", "geopoints", "GeoPointsFileOutput[path="},
                 {"a", "geopoints-xyv", "GeoPointsFileOutput[path="},
                 {"a", "geopoints-xy-vector", "GeoPointsFileOutput[path="},
                 {"a", "gmsh", "GmshOutput[path="},
                 {"a", "png", "PNGOutput[path="},
                 {"a.gpt", "extension", "GeoPointsFileOutput[path="},
             }) {
            param::SimpleParametrisation user;
            user.set("format", test.format);

            auto out = build(test.name, user);
            EXPECT(to_string(*out).find(test.print) == 0);
        }

        param::SimpleParametrisation user;
        user.set("format", "unknown");
        EXPECT_THROWS_AS(build("a", user), exception::SeriousBug);

        std::ostringstream list;
        output::MIROutputFactory::list(list);
        EXPECT(list.str().find("geopoints-xy-vector") != std::string::npos);
    }


    SECTION("by extension") {
        for (const auto& test : std::vector<test_t>{
                 {"a.grib", "", "GribFileOutput[path="},
                 {"a.unknown", "", "GribFileOutput[path="},
                 {"a", "", "GribFileOutput[path="},
                 {"a.gp", "", "GeoPointsFileOutput[path="},
                 {"a.gpt", "", "GeoPointsFileOutput[path="},
                 {"a.geo", "", "GeoPointsFileOutput[path="},
                 {"a.msh", "", "GmshOutput[path="},
                 {"a.png", "", "PNGOutput[path="},
             }) {
            auto out = build(test.name, param::SimpleParametrisation());
            EXPECT(to_string(*out).find(test.print) == 0);
        }
    }


    SECTION("geopoints (if latitudes or longitudes)") {
        for (const auto& key : std::vector<std::string>{"latitudes", "longitudes"}) {
            param::SimpleParametrisation user;
            user.set(key, "x");

            auto out = build("a.grib", user);
            EXPECT(to_string(*out).find("GeoPointsFileOutput[path=") == 0);
        }
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
