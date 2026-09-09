// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


CASE("Representation::fillJob(MIRJob&)") {
    std::vector<std::string> tests{
        "F16", "O16", "F21", "O21", "N16", "eORCA1_T",
    };

    for (const auto& test : tests) {
        repres::RepresentationHandle repres = key::grid::Grid::lookup(test).representation();
        ASSERT(repres);

        api::MIRJob job;
        repres->fillJob(job);

        std::string grid;
        EXPECT(job.get("grid", grid));
        EXPECT(grid == R"({"grid":")" + test + R"("})");
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
