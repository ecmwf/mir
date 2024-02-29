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


#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/compare/FieldComparator.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRList : MIRTool {
    MIRList(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("json", "JSON output"));
        compare::FieldComparator::addOptions(options_);
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n" << "Usage: " << tool << " ..." << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override;
};


void MIRList::execute(const eckit::option::CmdArgs& args) {
    compare::FieldComparator comparator(args);

    bool json = false;
    args.get("json", json);

    if (json) {
        eckit::JSON j(std::cout);
        j.startList();
        for (size_t i = 0; i < args.count(); i++) {
            comparator.json(j, args(i));
        }
        j.endList();
    }
    else {

        for (size_t i = 0; i < args.count(); i++) {
            Log::info() << args(i) << " ==> " << std::endl;
            comparator.list(args(i));
        }
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRList tool(argc, argv);
    return tool.start();
}
