/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "mir/compare/FieldComparator.h"
#include "mir/tools/MIRTool.h"


class MIRList : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs &args);

    void usage(const std::string &tool) const;

    int minimumPositionalArguments() const {
        return 1;
    }

public:

    // -- Contructors

    MIRList(int argc, char **argv) :
        mir::tools::MIRTool(argc, argv) {
    }

};


void MIRList::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " ..."
            << std::endl;
}


void MIRList::execute(const eckit::option::CmdArgs &args) {

    mir::compare::FieldComparator comparator(args);

    for (size_t i = 0; i < args.count(); i++) {
        eckit::Log::info() << args(i) << " ==> " << std::endl;
        comparator.list(args(i));
    }

}


int main(int argc, char **argv) {
    MIRList tool(argc, argv);
    return tool.start();
}
