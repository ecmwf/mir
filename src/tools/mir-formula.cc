// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sstream>

#include "mir/action/context/Context.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Formula.h"
#include "mir/util/FormulaParser.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRFormula : MIRTool {
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


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRFormula tool(argc, argv);
    return tool.start();
}
