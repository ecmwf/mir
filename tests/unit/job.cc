/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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
