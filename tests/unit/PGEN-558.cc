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


#include "eckit/testing/Test.h"

#include <memory>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/input/GridSpecInput.h"
#include "mir/output/EmptyOutput.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Log.h"

// define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir::tests::unit {


CASE("PGEN-558") {
    auto& log = Log::info();
    output::EmptyOutput output;


    SECTION("unit test") {
        repres::RepresentationHandle rep(new repres::gauss::reduced::ReducedOctahedral(64));
        ASSERT(rep);
        log << *rep << std::endl;

        repres::RepresentationHandle cropped(rep->croppedRepresentation({73, 80, 70, 280}));
        ASSERT(cropped);
        log << *cropped << std::endl;

        const auto& crop = cropped->boundingBox();

        EXPECT(crop.west() == Longitude{80});
        EXPECT(crop.east() == Longitude{280});
    }


    mir::api::MIRJob job;
    job.set("grid", "O64");
    job.set("interpolation", "nn");
    job.set("dont-compress-plan", true);
    job.set("area", std::vector<double>{73, 80, 70, 280});


    SECTION("Representation::validate: {grid: F64} -> {grid: O64} -> {grid: O64, area: ...}") {
        std::unique_ptr<input::MIRInput> input(new input::GridSpecInput("{grid: O64}", true));
        job.execute(*input, output);
    }


    SECTION("Representation::validate: {grid: F64} -> {grid: O64} -> {grid: O64, area: ...}") {
        std::unique_ptr<input::MIRInput> input(new input::GridSpecInput("{grid: F64}", true));
        job.execute(*input, output);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
