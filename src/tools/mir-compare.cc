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


#include "eckit/option/CmdArgs.h"

#include "mir/compare/FieldComparator.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRCompare : MIRTool {
    MIRCompare(int argc, char** argv) : MIRTool(argc, argv) { compare::FieldComparator::addOptions(options_); }

    int numberOfPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Usage: " << tool << " [options] file1 file2" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
};


void MIRCompare::execute(const eckit::option::CmdArgs& args) {

    // Straightforward two-file comparison
    compare::FieldComparator c(args);

    c.compare(args(0), args(1));
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRCompare tool(argc, argv);
    return tool.start();
}
