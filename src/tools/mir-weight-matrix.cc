/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   Dec 2016


#include <fstream>

#include "eckit/io/FileHandle.h"
#include "eckit/log/Bytes.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/caching/matrix/SharedMemoryLoader.h"
#include "mir/method/WeightMatrix.h"
#include "mir/tools/MIRTool.h"


class MIRWeightMatrix : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

public:

    // -- Contructors

    MIRWeightMatrix(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<bool>("load", "Load file into shared memory. If file is already loaded, does nothing."));
        options_.push_back(new SimpleOption<bool>("unload", "Load file into shared memory. If file is not loaded, does nothing."));

        options_.push_back(new SimpleOption<eckit::PathName>("dump", "Matrix dump (needs --load)"));
    }

};


void MIRWeightMatrix::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " [--load] [--unload] [--dump=path] <path>"
            << std::endl;
}


void MIRWeightMatrix::execute(const eckit::option::CmdArgs& args) {
    using mir::method::WeightMatrix;

    std::string path(args(0));

    if (args.has("load") || args.has("dump")) {

        mir::caching::matrix::SharedMemoryLoader::loadSharedMemory(path);

        WeightMatrix W(new mir::caching::matrix::SharedMemoryLoader("shmem", path));

        eckit::linalg::SparseMatrix& spm = W;
        eckit::Log::info()
                << spm
                << " memory " << W.footprint() << " bytes"
                << std::endl;

        if(args.has("dump")) {

            std::string s;
            args.get("dump", s);

            eckit::PathName file(s);
            if (file.exists()) {
                throw eckit::WriteError("File " + s + " exists");
            }

            std::ofstream out(file.asString().c_str());
            if (!out) {
                throw eckit::CantOpenFile(file);
            }

            W.dump(out);

            out.close();
            if (out.bad()) {
                throw eckit::WriteError(file);
            }
        }

    }

    if (args.has("unload")) {
        mir::caching::matrix::SharedMemoryLoader::unloadSharedMemory(path);
    }
}


int main(int argc, char **argv) {
    MIRWeightMatrix tool(argc, argv);
    return tool.start();
}

