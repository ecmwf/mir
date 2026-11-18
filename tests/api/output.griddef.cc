// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>
#include <string>
#include <vector>

#include "eckit/filesystem/TmpDir.h"
#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GriddefInput.h"
#include "mir/output/GriddefOutput.h"
#include "mir/param/SimpleParametrisation.h"

#include "output.h"


namespace mir::api::tests {


CASE("GriddefOutput") {
    eckit::TmpDir dir;

    GridInput in("{grid: [10, 10], area: [10, 0, 0, 20]}");

    MIRJob job;
    job.set("caching", false);

    for (const auto& binary : {false, true}) {
        auto path = (dir / (binary ? "binary" : "text")).asString();

        {
            output_t out(binary ? static_cast<MIROutput*>(new output::GriddefBinaryOutput(path))
                                : new output::GriddefTextOutput(path));
            job.execute(*in, *out);

            MIROutput& base = *out;
            EXPECT(base.sameAs(output::GriddefTextOutput(path)));
            EXPECT(base.sameAs(output::GriddefBinaryOutput(path)));
            EXPECT_NOT(base.sameAs(output::GriddefTextOutput(path + "x")));

            param::SimpleParametrisation p;
            EXPECT_NOT(base.sameParametrisation(p, p));

            std::ostringstream str;
            EXPECT_NOT(base.printParametrisation(str, p));
            EXPECT_EQUAL(to_string(base), "GriddefOutput[path=" + path + ",binary=" + (binary ? "1" : "0") + "]");
        }

        std::vector<double> latitudes;
        std::vector<double> longitudes;
        input::GriddefInput::load(path, latitudes, longitudes);

        EXPECT(latitudes == std::vector<double>({10., 10., 10., 0., 0., 0.}));
        EXPECT(longitudes == std::vector<double>({0., 10., 20., 0., 10., 20.}));
    }

    auto lines = read_lines((dir / "text").asString());
    EXPECT_EQUAL(lines.size(), 6);
    EXPECT_EQUAL(lines[0], "10.000000 0.000000");
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
