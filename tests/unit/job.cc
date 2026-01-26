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
    struct test_t {
        std::string grid;
        std::string spec;
    };

    std::vector<test_t> tests{
        test_t{"F16",  //
               R"({"grid":"F16"})"},

        {"O16",  //
         R"({"grid":"O16","pl":[20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,80,76,72,68,64,60,56,52,48,44,40,36,32,28,24,20]})"},

        {"F21",  //
         R"({"grid":"F21"})"},

        {"O21",  //
         R"({"grid":"O21","pl":[20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,96,100,100,96,92,88,84,80,76,72,68,64,60,56,52,48,44,40,36,32,28,24,20]})"},

        {"N16",  //
         R"({"grid":"N16","pl":[20,27,32,40,45,48,60,60,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,60,60,48,45,40,32,27,20]})"},

        {"eORCA1_T",  //
         R"({"grid":"eORCA1_T"})"},
    };

    for (const auto& test : tests) {
        repres::RepresentationHandle repres = key::grid::Grid::lookup(test.grid).representation();
        ASSERT(repres);

        api::MIRJob job;
        repres->fillJob(job);

        std::string grid;
        EXPECT(job.get("grid", grid));
        EXPECT(grid == test.grid);

        EXPECT(test.spec == job.json_str());
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
