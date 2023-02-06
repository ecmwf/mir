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

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"

#include "mir/action/calc/FormulaAction.h"
#include "mir/action/context/Context.h"
#include "mir/action/io/Save.h"
#include "mir/input/MIRInput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace tools {


struct MIRCompute : MIRTool {
    MIRCompute(int argc, char** argv) : MIRTool(argc, argv) {
        using str = eckit::option::SimpleOption<std::string>;

        options_.push_back(new str("input", "Input options (YAML)"));
        options_.push_back(new str("formula", "Formula using variables 'f'/'f1'/'f2'/..."));
        options_.push_back(new str("metadata", "Set eccodes keys to integer values (a=b,c=d,..)"));
    }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool
                    << " [--input={}] [--formula=f(f,f1,f2,...)] [--metadata=a=1,b=2,...)] input.grib output.grib"
                       "\n"
                       "Examples: "
                       "\n  % "
                    << tool
                    << " --formula=2+3*f input.grib output.grib"
                       "\n  % "
                    << tool
                    << " --formula=\"sqrt(f1*f1+f2^2);f1-f2;f1+f2\" --input=\"{multiDimensional: 2}\" "
                       "--metadata=paramId=1;paramId=2;paramId=3 input.grib output.grib"
                    << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};


void MIRCompute::execute(const eckit::option::CmdArgs& args) {
    const param::ConfigurationWrapper param(args);


    // create formula(s)/metadata(s)
    auto args_split = [&args](const std::string& arg) {
        std::string val;
        args.get(arg, val);
        return eckit::StringTools::split(";", val);
    };

    auto formulas = args_split("formula");
    ASSERT(!formulas.empty());

    auto metadatas = args_split("metadata");
    ASSERT(metadatas.empty() || metadatas.size() == formulas.size());


    // create input/output/save
    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(0), param));
    std::unique_ptr<output::MIROutput> output(output::MIROutputFactory::build(args(1), param));
    std::unique_ptr<action::Action> save(new action::io::Save(param, *output));


    // for each input (possibly more than one field at a time), process each formula/metadata
    while (input->next()) {
        for (size_t i = 0; i < formulas.size(); ++i) {
            util::MIRStatistics statistics;
            context::Context ctx(*input, statistics);

            param::RuntimeParametrisation run(param);
            run.set("formula", formulas[i]);
            run.set("formula.metadata", metadatas.empty() ? "" : metadatas[i]);

            std::unique_ptr<action::FormulaAction> formula(new action::FormulaAction(run));
            formula->perform(ctx);
            Log::info() << ctx << std::endl;

            save->perform(ctx);
        }
    }
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRCompute tool(argc, argv);
    return tool.start();
}
