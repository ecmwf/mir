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


#include <sstream>

#include "mir/action/context/Context.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Formula.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/Log.h"


namespace mir {
namespace tools {


struct MIRFormula : tools::MIRTool {
    using MIRTool::MIRTool;

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
};


void MIRFormula::execute(const eckit::option::CmdArgs& /*args*/) {
    // std::istringstream in("sqrt(-(-2 + 3 - 4   - 5*10/2))");
    std::istringstream in("2 ^ 10");
    util::FormulaParser p(in);

    param::SimpleParametrisation param;

    util::Formula* f = p.parse(param);
    Log::info() << (*f) << std::endl;

    context::Context ctx;
    f->perform(ctx);

    Log::info() << ctx << std::endl;
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRFormula tool(argc, argv);
    return tool.start();
}
