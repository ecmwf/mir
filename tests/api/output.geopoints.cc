// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/output/GeoPointsFileOutputXYV.h"
#include "mir/output/GeoPointsFileOutputXYVector.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "output.h"


namespace mir::api::tests {


CASE("GeoPointsFileOutputXYV") {
    eckit::TmpDir dir;
    auto path = (dir / "out.gpt").asString();

    GridInput in("{grid: [10, 10], area: [10, 0, 0, 20]}");
    in.metadata.set("stream", "oper");
    in.metadata.set("missing_value", 1.);

    MIRJob job;
    job.set("caching", false);


    SECTION("text") {
        {
            output::GeoPointsFileOutputXYV out(path);
            job.execute(*in, out);

            // only once
            EXPECT_THROWS_AS(job.execute(*in, out), exception::SeriousBug);
        }

        auto lines = read_lines(path);
        EXPECT(lines.size() >= 4);
        EXPECT_EQUAL(lines[0], "#GEO");
        EXPECT_EQUAL(lines[1], "#FORMAT XYV");
        EXPECT_EQUAL(lines[2], "# stream=oper");
        EXPECT_EQUAL(lines[3], "#DATA");

        // 2x3 points (lon lat value), missing value (1) skipped
        EXPECT_EQUAL(lines.size(), 4 + 6 - 1);
        EXPECT_EQUAL(lines[4], "0 10 0");
        EXPECT_EQUAL(lines[5], "20 10 2");

        // the file reads back
        input::GeoPointsFileInput geo(path);
        EXPECT_EQUAL(geo.values().size(), 5);
        EXPECT_EQUAL(geo.values().back(), 5.);
    }


    SECTION("binary") {
        {
            output::GeoPointsFileOutputXYV out(path, true);
            job.execute(*in, out);
        }

        // the file reads back (missing values included)
        input::GeoPointsFileInput geo(path);
        EXPECT_EQUAL(geo.values().size(), 6);
        EXPECT_EQUAL(geo.latitudes().front(), 10.);
        EXPECT_EQUAL(geo.longitudes().back(), 20.);
        EXPECT_EQUAL(geo.values().back(), 5.);

        input::MIRInput& base = geo;
        std::string stream;
        EXPECT(base.parametrisation().get("stream", stream) && stream == "oper");
        EXPECT(base.next());
        EXPECT_EQUAL(base.field().values(0).size(), 6);
    }


    SECTION("comparisons") {
        output::GeoPointsFileOutputXYV out(path);
        MIROutput& base = out;

        EXPECT(base.sameAs(output::GeoPointsFileOutputXYV(path)));
        EXPECT_NOT(base.sameAs(output::GeoPointsFileOutputXYV(path, true)));
        EXPECT_NOT(base.sameAs(output::EmptyOutput()));
        EXPECT_EQUAL(to_string(base), "GeoPointsFileOutput[path=" + path + "]");

        param::SimpleParametrisation p;
        EXPECT(base.sameParametrisation(p, p));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, p));
    }
}


CASE("GeoPointsFileOutputXYVector") {
    eckit::TmpDir dir;
    auto path = (dir / "out.gpt").asString();

    GridInput in("{grid: [10, 10], area: [10, 0, 0, 10]}", 2);

    MIRJob job;
    job.set("caching", false).set("date", "20250101");

    {
        output::GeoPointsFileOutputXYVector out(path);
        job.execute(*in, out);
    }

    auto lines = read_lines(path);
    EXPECT_EQUAL(lines.size(), 4 + 4);
    EXPECT_EQUAL(lines[1], "#FORMAT XY_VECTOR");
    EXPECT_EQUAL(lines[4], "10 0 0 20250101 0 0 100");
    EXPECT_EQUAL(lines[7], "0 10 0 20250101 0 3 103");

    // the file reads back (the last column as value)
    input::GeoPointsFileInput geo(path);
    EXPECT_EQUAL(geo.values().size(), 4);
    EXPECT_EQUAL(geo.values().front(), 100.);
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
