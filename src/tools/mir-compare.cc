/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/tools/MIRTool.h"
#include "eckit/log/Log.h"
#include "mir/compare/Comparator.h"
#include "eckit/option/CmdArgs.h"


class MIRCompare : public mir::tools::MIRTool {
protected:

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual int numberOfPositionalArguments() const {
        return 2;
    }

    virtual void usage(const std::string& tool) const;

public:

    MIRCompare(int argc, char **argv);

};


MIRCompare::MIRCompare(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
    mir::compare::Comparator::addOptions(options_);
}


void MIRCompare::usage(const std::string& tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " [options] file1 file2"
            << std::endl;
}

void MIRCompare::execute(const eckit::option::CmdArgs& args) {

    // Straightforward two-file comparison
    mir::compare::Comparator c(args);

    c.compare(args(0), args(1));
}

int main(int argc, char **argv) {
    MIRCompare tool(argc, argv);
    return tool.start();
}

