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
#include <vector>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/calc/FormulaAction.h"
#include "mir/action/context/Context.h"
#include "mir/action/io/Save.h"
#include "mir/input/MIRInput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::tools {


struct MIRCompute : MIRTool {
    MIRCompute(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("input", "Input options (YAML)"));
        options_.push_back(new eckit::option::VectorOption<std::string>(
            "formula", "Formula(s) on variables f/f1/f2/... (formula;formula;...)", 0, ";"));
        options_.push_back(new eckit::option::VectorOption<long>("param-id", "GRIB paramId(s) (1;2;...)", 0, ";"));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool
                    << " --formula=f(f,f1,f2,...)[;g(f,f1,f2,...)[;...]] [--param-id=1[;2[;...]]] [--input={}] "
                       "input.grib output.grib"
                       "\n"
                       "Examples: "
                       "\n  % "
                    << tool
                    << " --formula=2+3*f input.grib output.grib"
                       "\n  % "
                    << tool
                    << " --formula=\"sqrt(f1*f1+f2^2);f1-f2;f1+f2\" --input=\"{multiDimensional: 2}\" "
                       "--param-id=1;2;3 input.grib output.grib"
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRCompute::execute(const eckit::option::CmdArgs& args) {
    const param::ConfigurationWrapper param(args);


    // arguments
    std::vector<std::string> formulas;
    args.get("formula", formulas);
    ASSERT(!formulas.empty());

    std::vector<long> paramids;
    args.get("param-id", paramids);
    ASSERT(paramids.empty() || paramids.size() == formulas.size());


    // create input/output
    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(0), param));
    std::unique_ptr<output::MIROutput> output(output::MIROutputFactory::build(args(1), param));


    // for each input (possibly more than one field at a time), process each formula/metadata
    while (input->next()) {
        for (size_t i = 0; i < formulas.size(); ++i) {
            util::MIRStatistics statistics;
            context::Context ctx(*input, statistics);

            // run-time parametrisation
            param::SimpleParametrisation user;
            user.set("formula", formulas[i]);
            user.set("formula.metadata", paramids.empty() ? "" : "paramId=" + std::to_string(paramids[i]));

            const param::DefaultParametrisation defaults;
            std::unique_ptr<param::MIRParametrisation> param(
                new param::CombinedParametrisation(user, input->parametrisation(), defaults));

            std::unique_ptr<action::FormulaAction> formula(new action::FormulaAction(*param));
            formula->perform(ctx);
            Log::info() << ctx << std::endl;

            std::unique_ptr<action::Action> save(new action::io::Save(*param, *output));
            save->perform(ctx);
        }
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRCompute tool(argc, argv);
    return tool.start();
}
