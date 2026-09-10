// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/tools/MIRTool.h"

#include "eckit/option/CmdArgs.h"

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
    atlas::Library::instance().initialise(args);
}


void MIRTool::finish(const eckit::option::CmdArgs& /*unused*/) {
    atlas::Library::instance().finalise();
}


}  // namespace mir::tools
