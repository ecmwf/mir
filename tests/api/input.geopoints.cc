// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"

#include "input.h"


namespace mir::api::tests {


CASE("GeoPointsFileInput") {
    struct test_t {
        std::string path;
        size_t size;
        double lat;
        double lon;
        double value;
    };


    SECTION("text formats") {
        for (const auto& test : std::vector<test_t>{
                 {"geopoints-standard.gpt", 0, 36.15, -5.35, 300.9},
                 {"geopoints-format-xyv.gpt", 0, 36.15, -5.35, 300.9},
                 {"geopoints-format-ncols.gpt", 0, 32.55, 35.85, 75.},
                 {"geopoints-format-xy-vector.gpt", 0, 80., 0., -8.3126},
                 {"geopoints-format-polar-vector.gpt", 3, 50.97, 6.05, 90.},
             }) {
            Log::info() << "'" << test.path << "'" << std::endl;

            input::GeoPointsFileInput geo(test.path);
            MIRInput& in = geo;

            EXPECT(in.next());
            EXPECT_EQUAL(in.dimensions(), 1);
            EXPECT_NOT(in.next());

            EXPECT(!geo.latitudes().empty());
            EXPECT_EQUAL(geo.latitudes().size(), geo.longitudes().size());
            EXPECT_EQUAL(geo.latitudes().size(), geo.values().size());
            if (test.size > 0) {
                EXPECT_EQUAL(geo.values().size(), test.size);
            }

            EXPECT_EQUAL(geo.latitudes().front(), test.lat);
            EXPECT_EQUAL(geo.longitudes().front(), test.lon);
            EXPECT_EQUAL(geo.values().front(), test.value);
            EXPECT(geo.footprint() > 0);

            std::string gridType;
            EXPECT(in.parametrisation().get("gridType", gridType) && gridType == "unstructured_grid");

            auto field = in.field();
            EXPECT(field.values(0) == geo.values());

            EXPECT(in.sameAs(*build(test.path)));
            EXPECT(to_string(in).find("GeoPointsFileInput[path=" + test.path) == 0);
        }

        // metadata
        input::GeoPointsFileInput geo("geopoints-standard.gpt");
        auto metadata = to_string(static_cast<MIRInput&>(geo).parametrisation());
        Log::info() << "metadata: " << metadata << std::endl;
        EXPECT(metadata.find("12004") != std::string::npos);
    }


    SECTION("multiple fields") {
        eckit::TmpDir dir;
        auto path = (dir / "multi.gpt").asString();
        write_file(path,
                   "#GEO\n#FORMAT XYV\n#DATA\n1 2 3\n4 5 6\n"
                   "#GEO\n#FORMAT XYV\n#DATA\n7 8 9\n");

        EXPECT_THROWS_AS(input::GeoPointsFileInput{path}, exception::SeriousBug);
        EXPECT_THROWS_AS(input::GeoPointsFileInput(path, 2), exception::SeriousBug);

        input::GeoPointsFileInput first(path, 0);
        EXPECT_EQUAL(first.values().size(), 2);
        EXPECT_EQUAL(first.values().front(), 3.);

        input::GeoPointsFileInput second(path, 1);
        EXPECT_EQUAL(second.values().size(), 1);
        EXPECT_EQUAL(second.values().front(), 9.);

        // the selected field only
        MIRInput& in = second;
        EXPECT_EQUAL(in.dimensions(), 1);
        EXPECT(in.next());
        EXPECT_NOT(in.next());
    }


    SECTION("errors") {
        eckit::TmpDir dir;
        auto path = (dir / "empty.gpt").asString();
        write_file(path, "#NOTHING\n");

        EXPECT_THROWS_AS(input::GeoPointsFileInput{path}, exception::SeriousBug);
        EXPECT_THROWS_AS(input::GeoPointsFileInput{"does-not-exist.gpt"}, exception::CantOpenFile);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
