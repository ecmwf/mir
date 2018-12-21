/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <string>

#include "eckit/log/Log.h"
#include "eckit/system/Library.h"

#include "mir/api/Atlas.h"
#include "mir/tools/MIRTool.h"

class MIRInfo : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string&) const {}

public:
    MIRInfo(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {}
};

void MIRInfo::execute(const eckit::option::CmdArgs&) {
    using eckit::system::Library;

    for (const auto& lib_name : Library::list()) {
        const Library& lib = Library::lookup(lib_name);
        eckit::Log::info() << lib.name() << " " << lib.version() << " git-sha1:" << lib.gitsha1(8) << std::endl;
    }
}

int main(int argc, char** argv) {
    MIRInfo tool(argc, argv);
    return tool.start();
}
