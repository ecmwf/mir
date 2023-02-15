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

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/system/Library.h"
#include "eckit/system/LibraryManager.h"

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::tools {


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

    constexpr size_t sha1len = 8;
    eckit::option::CmdArgs args(&tools::usage, options_, numberOfPositionalArguments(), minimumPositionalArguments());

    if (args.has("version")) {
        using eckit::system::LibraryManager;

        for (const auto& lib_name : LibraryManager::list()) {
            const auto& lib = LibraryManager::lookup(lib_name);
            Log::info() << lib.name() << " " << lib.version() << " git-sha1:" << lib.gitsha1(sha1len)
                        << " home:" << lib.libraryHome() << std::endl;
        }

        Log::info() << "eccodes " << ECCODES_VERSION_STR
                    << " git-sha1:" << std::string(codes_get_git_sha1()).substr(0, sha1len) << std::endl;
    }

    init(args);
    execute(args);
    finish(args);
}

void MIRTool::init(const eckit::option::CmdArgs& args) {
    atlas::Library::instance().initialise(args);
}

void MIRTool::finish(const eckit::option::CmdArgs& /*unused*/) {
    atlas::Library::instance().finalise();
}


}  // namespace mir::tools
