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


#include "mir/action/context/Context.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Formula.h"
#include "mir/util/FormulaParser.h"


class MIRFormula : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

public:
    // -- Constructors

    MIRFormula(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {}
};


void MIRFormula::usage(const std::string& tool) const {
    eckit::Log::info() << "\n"
                          "Usage: "
                       << tool << std::endl;
}


void MIRFormula::execute(const eckit::option::CmdArgs&) {
    // std::istringstream in("sqrt(-(-2 + 3 - 4   - 5*10/2))");
    std::istringstream in("2 ^ 10");
    mir::util::FormulaParser p(in);

    mir::param::SimpleParametrisation param;

    mir::util::Formula* f = p.parse(param);
    eckit::Log::info() << (*f) << std::endl;

    mir::context::Context ctx;
    f->perform(ctx);

    eckit::Log::info() << ctx << std::endl;
}


int main(int argc, char** argv) {
    MIRFormula tool(argc, argv);
    return tool.start();
}
