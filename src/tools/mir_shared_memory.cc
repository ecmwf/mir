/*
 * (C) Copyright 1996-2015 ECMWF.
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

#include "eckit/runtime/Tool.h"

#include "mir/caching/SharedMemoryLoader.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/option/SimpleOption.h"

using mir::param::option::Option;
using mir::param::option::SimpleOption;


class MIRSharedMemory : public eckit::Tool {

    virtual void run();
    static void usage(const std::string &tool);

  public:
    MIRSharedMemory(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void MIRSharedMemory::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " --load=file | --unload=file" << std::endl
            ;

}

void MIRSharedMemory::run() {


    std::vector<const Option*> options;
    options.push_back(new SimpleOption<eckit::PathName>("load", "Load file into shared memory. If file already loaded, does nothing."));
    options.push_back(new SimpleOption<eckit::PathName>("unload", "Load file into shared memory. If file already loaded, does nothing."));

    mir::param::MIRArgs args(&usage, 0, options);

    std::string path;

    if (args.get("load", path)) {
        mir::caching::SharedMemoryLoader::loadSharedMemory(path);
    }

    if (args.get("unload", path)) {
        mir::caching::SharedMemoryLoader::unloadSharedMemory(path);
    }

}


int main( int argc, char **argv ) {
    MIRSharedMemory tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}

