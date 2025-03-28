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


// #include <cctype>
// #include <fstream>
// #include <map>
// #include <set>
// #include <sstream>
// #include <string>

#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/api/mir_config.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRConfig : MIRTool {
    MIRConfig(int argc, char** argv) : MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("json", "Display configuration in JSON"));
    }

    int minimumPositionalArguments() const override { return 0; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nUsage: " << tool << " [--json]" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& args) override {
        bool json = false;
        args.get("json", json);

        if (json) {
            eckit::JSON out(Log::info());
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

        /*
        Build:
          build type      : Release
          timestamp       : 20250304004606
          op. system      : Darwin-24.2.0 (macosx.64)
          processor       : arm64
          sources         : /tmp/eccodes-20250303-30948-xo4eyj/eccodes-2.40.0-Source
          c++ compiler    : AppleClang 16.0.0.16000026
          flags         :  -pipe

        Features:
          FORTRAN             : ON
          AEC                 : ON
          MEMFS               : OFF
          ECCODES_THREADS     : ON
          ECCODES_OMP_THREADS : OFF
          JPG                 : ON
          PNG                 : ON
          NETCDF              : ON
         */
    };
};


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRConfig tool(argc, argv);
    return tool.start();
}
