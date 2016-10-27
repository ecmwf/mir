/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "mir/caching/legendre/SharedMemoryLoader.h"
#include "mir/tools/MIRTool.h"


class MIRSharedMemory : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

public:

    // -- Contructors

    MIRSharedMemory(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new SimpleOption<eckit::PathName>("load", "Load file into shared memory. If file already loaded, does nothing."));
        options_.push_back(new SimpleOption<eckit::PathName>("unload", "Load file into shared memory. If file already loaded, does nothing."));
    }

};


void MIRSharedMemory::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " [--load=file] [--unload=file]"
            << std::endl;
}


void MIRSharedMemory::execute(const eckit::option::CmdArgs& args) {
    std::string path;

    if (args.get("load", path)) {
        mir::caching::legendre::SharedMemoryLoader::loadSharedMemory(path);
    }

    if (args.get("unload", path)) {
        mir::caching::legendre::SharedMemoryLoader::unloadSharedMemory(path);
    }
}


int main(int argc, char **argv) {
    MIRSharedMemory tool(argc, argv);
    return tool.start();
}

