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

#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"

#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/caching/legendre/SharedMemoryLoader.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir {
namespace tools {


struct MIRLoadLegendre : MIRTool {
    MIRLoadLegendre(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(
            new SimpleOption<bool>("load", "Load file into memory. If file is already loaded, does nothing."));
        options_.push_back(new SimpleOption<bool>(
            "unload",
            "Unload file from memory. If file is not loaded, or loader does not employ shmem, does nothing."));
        options_.push_back(new SimpleOption<bool>("wait", "After load/unload, wait for user input before exiting."));
        options_.push_back(new FactoryOption<caching::legendre::LegendreLoaderFactory>(
            "legendre-loader", "Select how to load Legendre coefficients in memory"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                    << "Usage: " << tool << " [--load] [--unload] <path>" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


void display(Log::Channel& out, caching::legendre::LegendreLoader* loader, const std::string& path) {
    ASSERT(loader);

    // clang-format off
    out << "\n" "path:\t'" << path << "'"
        << "\n" "size:\t" << Log::Bytes(loader->size())
        << "\n" "address:\t" << std::hex << loader->address() << std::dec
        << "\n" "inSharedMemory:\t" << std::boolalpha << loader->inSharedMemory() << std::noboolalpha
        << std::endl;
    // clang-format on
}


void MIRLoadLegendre::execute(const eckit::option::CmdArgs& args) {
    using caching::legendre::LegendreLoader;
    using caching::legendre::LegendreLoaderFactory;
    using caching::legendre::SharedMemoryLoader;

    Log::debug().setStream(std::cerr);
    Log::info().setStream(std::cerr);

    const param::ConfigurationWrapper param(args);

    bool load   = false;
    bool unload = false;
    bool wait   = false;
    param.get("load", load);
    param.get("unload", unload);
    param.get("wait", wait);

    if (load || unload) {
        for (const std::string& path : args) {

            if (!load) {
                Log::info() << "---"
                               "\n"
                               "unloadSharedMemory"
                            << std::endl;
                SharedMemoryLoader::unloadSharedMemory(path);
                continue;
            }

            Log::info() << "---"
                           "\n"
                           "load"
                        << std::endl;

            std::unique_ptr<LegendreLoader> loader(LegendreLoaderFactory::build(param, path));
            display(Log::info(), loader.get(), path);

            if (unload) {
                auto* shmLoader = dynamic_cast<SharedMemoryLoader*>(loader.get());
                if (shmLoader != nullptr) {
                    Log::info() << "---"
                                   "\n"
                                   "unload"
                                << std::endl;
                    SharedMemoryLoader::unloadSharedMemory(path);
                    display(Log::info(), loader.get(), path);
                }
            }
        }
    }

    if (wait) {
        Log::info() << "Press enter/return to continue..." << std::endl;
        std::cin.get();
    }
}


}  // namespace tools
}  // namespace mir


int main(int argc, char** argv) {
    mir::tools::MIRLoadLegendre tool(argc, argv);
    return tool.start();
}
