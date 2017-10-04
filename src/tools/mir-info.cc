/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <string>
#include "eckit/system/Library.h"
#include "mir/api/Atlas.h"
#include "mir/tools/MIRTool.h"


class MIRInfo : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);
    void usage(const std::string&) const {}
public:
    MIRInfo(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {}
};


void MIRInfo::execute(const eckit::option::CmdArgs&) {
    using eckit::system::Library;

    for (auto lib_name : Library::list()) {
        const Library& lib = Library::lookup(lib_name);
        eckit::Log::info() << lib.name() << " " << lib.version() << " git-sha1:" << lib.gitsha1(8) << std::endl;
    }

#ifdef ATLAS_HAVE_TRANS
    eckit::Log::info() << "transi " << transi_version() << " git-sha1:" << transi_git_sha1_abbrev(8) << std::endl;
    eckit::Log::info() << "trans " << trans_version() << " git-sha1:" << trans_git_sha1_abbrev(8) << std::endl;
#endif
}


int main(int argc, char **argv) {
    MIRInfo tool(argc, argv);
    return tool.start();
}

