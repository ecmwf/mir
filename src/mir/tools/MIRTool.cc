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

#include "mir/api/mir_config.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"


namespace mir::tools {


static MIRTool* instance_ = nullptr;


static void usage(const std::string& tool) {
    ASSERT(instance_);
    instance_->usage(tool);
}


MIRTool::MIRTool(int argc, char** argv) : eckit::Tool(argc, argv, "MIR_HOME") {
    ASSERT(instance_ == nullptr);
    instance_ = this;
}


void MIRTool::run() {
    eckit::option::CmdArgs args(&tools::usage, options_, numberOfPositionalArguments(), minimumPositionalArguments());

    init(args);
    execute(args);
    finish(args);
}


void MIRTool::init(const eckit::option::CmdArgs& args) {
#if mir_HAVE_ATLAS
    atlas::Library::instance().initialise(args);
#endif
}


void MIRTool::finish(const eckit::option::CmdArgs& /*unused*/) {
#if mir_HAVE_ATLAS
    atlas::Library::instance().finalise();
#endif
}


}  // namespace mir::tools
