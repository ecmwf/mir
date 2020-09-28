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


#include "mir/tools/MIRTool.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/system/Library.h"

#include "mir/api/Atlas.h"
#include "mir/api/mir_config.h"
#include "mir/util/Grib.h"


namespace mir {
namespace tools {


static MIRTool* instance_ = nullptr;


static void usage(const std::string& tool) {
    ASSERT(instance_);
    instance_->usage(tool);
}


MIRTool::MIRTool(int argc, char** argv) : eckit::Tool(argc, argv, "MIR_HOME") {
    ASSERT(instance_ == nullptr);
    instance_ = this;
    options_.push_back(new eckit::option::SimpleOption<bool>("version", "Display the version number"));
}


void MIRTool::run() {

    eckit::option::CmdArgs args(&mir::tools::usage, options_, numberOfPositionalArguments(),
                                minimumPositionalArguments());

    if (args.has("version")) {
        auto& log = eckit::Log::info();

        using eckit::system::Library;
        for (const auto& lib_name : Library::list()) {
            auto& lib = Library::lookup(lib_name);
            log << lib.name() << " " << lib.version() << " git-sha1:" << lib.gitsha1(8) << " home:" << lib.libraryHome()
                << std::endl;
        }

        log << "eccodes " << ECCODES_VERSION_STR << " git-sha1:" << std::string(codes_get_git_sha1()).substr(0, 8)
            << std::endl;
    }

    init(args);
    execute(args);
    finish(args);
}

void MIRTool::init(const eckit::option::CmdArgs& args) {
    atlas::Library::instance().initialise(args);
}

void MIRTool::finish(const eckit::option::CmdArgs&) {
    atlas::Library::instance().finalise();
}


}  // namespace tools
}  // namespace mir
