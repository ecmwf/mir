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


#include <ios>
#include <iostream>
#include <memory>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Bytes.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/caching/legendre/SharedMemoryLoader.h"
#include "mir/config/LibMir.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"


class MIRLoadLegendre : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const {
        eckit::Log::info() << "\n"
                           << "Usage: " << tool << " [--load] [--unload] <path>" << std::endl;
    }

    int minimumPositionalArguments() const { return 1; }

public:
    // -- Constructors

    MIRLoadLegendre(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "load", "Load file into memory. If file is already loaded, does nothing."));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "unload",
            "Unload file from memory. If file is not loaded, or loader does not employ shmem, does nothing."));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "wait", "After load/unload, wait for user input before exiting."));
        options_.push_back(new eckit::option::FactoryOption<mir::caching::legendre::LegendreLoaderFactory>(
            "legendre-loader", "Select how to load Legendre coefficients in memory"));
    }
};


void display(eckit::Channel& out, mir::caching::legendre::LegendreLoader* loader, std::string path) {
    ASSERT(loader);

    // clang-format off
    out << "\n" "path:\t'" << path << "'"
        << "\n" "size:\t" << eckit::Bytes(loader->size())
        << "\n" "address:\t" << std::hex << loader->address() << std::dec
        << "\n" "inSharedMemory:\t" << std::boolalpha << loader->inSharedMemory() << std::noboolalpha
        << std::endl;
    // clang-format on
}


void MIRLoadLegendre::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::caching::legendre;

    eckit::Log::debug<mir::LibMir>().setStream(std::cerr);
    eckit::Log::info().setStream(std::cerr);

    const mir::param::ConfigurationWrapper param(args);

    bool load   = false;
    bool unload = false;
    bool wait   = false;
    param.get("load", load);
    param.get("unload", unload);
    param.get("wait", wait);

    if (load || unload) {
        for (std::string path : args) {

            if (!load) {
                eckit::Log::info() << "---" "\n" "unloadSharedMemory" << std::endl;
                SharedMemoryLoader::unloadSharedMemory(path);
                continue;
            }

            eckit::Log::info() << "---" "\n" "load" << std::endl;

            std::unique_ptr<LegendreLoader> loader(LegendreLoaderFactory::build(param, path));
            display(eckit::Log::info(), loader.get(), path);

            if (unload) {
                auto shmLoader = dynamic_cast<SharedMemoryLoader*>(loader.get());
                if (shmLoader) {
                    eckit::Log::info() << "---" "\n" "unload" << std::endl;
                    shmLoader->unloadSharedMemory(path);
                    display(eckit::Log::info(), loader.get(), path);
                }
            }
        }
    }

    if (wait) {
        eckit::Log::info() << "Press enter/return to continue..." << std::endl;
        std::cin.get();
    }
}


int main(int argc, char** argv) {
    MIRLoadLegendre tool(argc, argv);
    return tool.start();
}
