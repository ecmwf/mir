// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/output/PNGOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"

#include "output.h"


namespace mir::api::tests {


CASE("PNGOutput") {
    eckit::TmpDir dir;
    auto path = (dir / "out.png").asString();

    GridInput in("{grid: [10, 10], area: [10, 0, -10, 20]}");

    MIRJob job;
    job.set("caching", false);

    const std::string magic("\x89PNG", 4);


    SECTION("encoders") {
        size_t i = 0;
        for (const auto& encoder :
             std::vector<std::string>{"8-bit/g", "8-bit/ga", "16-bit/g", "16-bit/ga", "8-bit/rgb", "8-bit/rgba"}) {
            auto p = (dir / ("out." + std::to_string(i++) + ".png")).asString();

            MIRJob j;
            j.set("caching", false).set("png-output-encoder", encoder).set("png-output-minmax", 0., 8.);

            output::PNGOutput out(p);
            j.execute(*in, out);

            EXPECT_EQUAL(read_bytes(p, 4), magic);
        }
    }


    SECTION("missing values, existing files") {
        in.metadata.set("missing_value", 0.);

        for (size_t i = 0; i < 2; ++i) {
            output::PNGOutput out(path);
            job.execute(*in, out);
        }

        EXPECT_EQUAL(read_bytes(path, 4), magic);
        EXPECT_EQUAL(read_bytes(path + ".0001", 4), magic);
    }


    SECTION("comparisons, errors") {
        output::PNGOutput out(path);
        MIROutput& base = out;

        EXPECT(base.sameAs(out));
        EXPECT_NOT(base.sameAs(output::PNGOutput(path)));
        EXPECT_EQUAL(to_string(base), "PNGOutput[path=" + path + "]");

        param::SimpleParametrisation p;
        EXPECT(base.sameParametrisation(p, p));

        std::ostringstream str;
        EXPECT_NOT(base.printParametrisation(str, p));

        MIRJob unknown;
        unknown.set("caching", false).set("png-output-encoder", "unknown");
        EXPECT_THROWS_AS(unknown.execute(*in, out), exception::SeriousBug);

        GridInput o16("{grid: O16}");
        EXPECT_THROWS_AS(job.execute(*o16, out), exception::UserError);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
