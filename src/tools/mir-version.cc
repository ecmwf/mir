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


#include <string>

#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/api/mir_config.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRVersion : MIRTool {
    MIRVersion(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("info", "Extra information"));
        options_.push_back(new eckit::option::SimpleOption<bool>("json", "Display in JSON"));
    }

    int minimumPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nUsage: " << tool << " [--json]" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        bool info = false;
        args.get("info", info);

        bool json = false;
        args.get("json", json);

        if (json) {
            eckit::JSON out(Log::info());

            if (!info) {
                out.startObject();
                out << "version" << mir_version();
                out.endObject();
                return;
            }

            out.startObject();

            out << "version" << mir_version();
            out << "git-sha1" << mir_git_sha1();

            out << "build";
            out.startObject();
            out << "type" << MIR_BUILD_TYPE;
            out << "timestamp" << MIR_BUILD_TIMESTAMP;
            out << "op. system" << (MIR_OS_NAME + std::string(" (") + MIR_OS_STR + std::string(")"));
            out << "processor" << MIR_SYS_PROCESSOR;
            out << "sources" << MIR_DEVELOPER_SRC_DIR;
            out << "c++ compiler" << (MIR_CXX_COMPILER_ID + std::string(" ") + MIR_CXX_COMPILER_VERSION);
            out << "flags" << MIR_CXX_FLAGS;
            out.endObject();

            out << "features";
            out.startObject();
            out << "HAVE_ATLAS" << HAVE_ATLAS;
            out << "HAVE_ECKIT_GEO" << HAVE_ECKIT_GEO;
            out << "HAVE_NETCDF" << HAVE_NETCDF;
            out << "HAVE_PNG" << HAVE_PNG;
            out << "HAVE_PROJ" << HAVE_PROJ;
            out << "HAVE_OMP" << HAVE_OMP;
            out << "HAVE_TESSELATION" << HAVE_TESSELATION;
            out.endObject();

            out.endObject();
            return;
        }

        auto& out = Log::info();

        if (!info) {
            out << mir_version() << std::endl;
            return;
        }

        out << "mir version " << mir_version() << ", git-sha1 " << mir_git_sha1() << '\n';

        out << "\nBuild:";
        out << "\n  type         : " << MIR_BUILD_TYPE;
        out << "\n  timestamp    : " << MIR_BUILD_TIMESTAMP;
        out << "\n  op. system   : " << (MIR_OS_NAME + std::string(" (") + MIR_OS_STR + std::string(")"));
        out << "\n  processor    : " << MIR_SYS_PROCESSOR;
        out << "\n  sources      : " << MIR_DEVELOPER_SRC_DIR;
        out << "\n  c++ compiler : " << (MIR_CXX_COMPILER_ID + std::string(" ") + MIR_CXX_COMPILER_VERSION);
        out << "\n  flags        : " << MIR_CXX_FLAGS;
        out << '\n';

        out << "\nFeatures:";
        out << "\n  HAVE_ATLAS       : " << HAVE_ATLAS;
        out << "\n  HAVE_ECKIT_GEO   : " << HAVE_ECKIT_GEO;
        out << "\n  HAVE_NETCDF      : " << HAVE_NETCDF;
        out << "\n  HAVE_PNG         : " << HAVE_PNG;
        out << "\n  HAVE_PROJ        : " << HAVE_PROJ;
        out << "\n  HAVE_OMP         : " << HAVE_OMP;
        out << "\n  HAVE_TESSELATION : " << HAVE_TESSELATION;
        out << std::endl;
    };
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRVersion tool(argc, argv);
    return tool.start();
}
