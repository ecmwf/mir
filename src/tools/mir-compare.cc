// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/exception/Exceptions.h"
#include "eckit/option/CmdArgs.h"

#include "mir/compare/FieldComparator.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Grib.h"
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


void mir_compare_assertion(const char* message) {
    throw eckit::SeriousBug(message);
}


int main(int argc, char** argv) {
    codes_set_codes_assertion_failed_proc(&mir_compare_assertion);

    mir::tools::MIRCompare tool(argc, argv);
    return tool.start();
}
