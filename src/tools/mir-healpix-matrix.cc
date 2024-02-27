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
#include "eckit/option/SimpleOption.h"

#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRHEALPixMatrix : MIRTool {
    MIRHEALPixMatrix(int argc, char** argv) : MIRTool(argc, argv) { using eckit::option::SimpleOption; }

    int numberOfPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override { Log::info() << "\n" << "Usage: " << tool << std::endl; }

    void execute(const eckit::option::CmdArgs& args) override;
};


void MIRHEALPixMatrix::execute(const eckit::option::CmdArgs& args) {
    const param::ConfigurationWrapper param(args);

    NOTIMP;  // TODO
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRHEALPixMatrix tool(argc, argv);
    return tool.start();
}
