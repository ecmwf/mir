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


#include <memory>
#include <sstream>

#include "eckit/testing/Test.h"

#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"


namespace mir::tests::unit {


CASE("PGEN-412") {
    // setup input/output
    param::SimpleParametrisation field;
    field.set("input",
              "{"
              "artificialInput:constant,"
              "constant:0.,"
              "spectral:true,"
              "truncation:1279,"
              "gridType:sh,"
              "packing:spectral_complex,"
              "edition:2,"
              "accuracy:24"
              "}");

    std::unique_ptr<input::MIRInput> in(input::MIRInputFactory::build("constant", field));
    std::unique_ptr<output::MIROutput> out(new output::GribFileOutput(""));


    auto graph_dump = [](input::MIRInput& in, output::MIROutput& out, const api::MIRJob& mj1, const api::MIRJob& mj2) {
        // prepare ActionPlan(s)
        std::unique_ptr<action::Job> aj1(new action::Job(mj1, in, out, false));
        std::unique_ptr<action::Job> aj2(new action::Job(mj2, in, out, false));

        // coallesce ActionPlan(s)
        action::ActionGraph graph;
        graph.add(aj1->plan(), nullptr);
        graph.add(aj2->plan(), nullptr);

        std::ostringstream ss;
        graph.dump(ss, 1);
        return ss.str();
    };


    SECTION("GribOutput::set()") {
        api::MIRJob mj1;
        mj1.set("accuracy", 12);

        api::MIRJob mj2;
        mj2.set("accuracy", 16);

        auto result = graph_dump(*in, *out, mj1, mj2);
        Log::info() << "ActionGraph:\n" << result << std::endl;

        const auto* reference =
            "   Set[accuracy=12,output=GribFileOutput[path=]]\n"
            "   Set[accuracy=16,output=GribFileOutput[path=]]\n";

        EXPECT(result == reference);
    }


    SECTION("GribOutput::save()") {
        api::MIRJob mj1;
        mj1.set("accuracy", 12).set("truncation", 20);

        api::MIRJob mj2;
        mj2.set("accuracy", 16).set("truncation", 20);

        auto result = graph_dump(*in, *out, mj1, mj2);
        Log::info() << "ActionGraph:\n" << result << std::endl;

        const auto* reference =
            "   ShTruncate[truncation=20]\n"
            "      Save[accuracy=12,output=GribFileOutput[path=]]\n"
            "      Save[accuracy=16,output=GribFileOutput[path=]]\n";

        EXPECT(result == reference);
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
