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


#include <memory>

#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/filesystem/PathName.h"

#include "mir/config/LibMir.h"
#include "mir/grib/Config.h"
#include "mir/grib/Packing.h"
#include "mir/input/MIRInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::tools {


struct MIRConfigGrib : MIRTool {
    MIRConfigGrib(int argc, char** argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::SimpleOption;

        options_.push_back(new SimpleOption<size_t>("accuracy", "Number of bits per value"));
        options_.push_back(new FactoryOption<grib::Packing>("packing", "GRIB packing method"));
        options_.push_back(new SimpleOption<size_t>("edition", "GRIB edition number"));

        options_.push_back(new SimpleOption<std::string>(
            "grib-packing-gridded", "GRIB default gridded packing, on gridded/spectral conversions"));
        options_.push_back(new SimpleOption<std::string>(
            "grib-packing-spectral", "GRIB default spectral packing, on gridded/spectral conversions"));
        options_.push_back(
            new SimpleOption<bool>("grib-packing-always-set", "GRIB packing setting to default, unless overriden"));
        options_.push_back(
            new SimpleOption<bool>("grib-edition-conversion", "GRIB edition conversion on packing changes"));
    }

    int minimumPositionalArguments() const override { return 1; }

    void usage(const std::string& tool) const override {
        Log::info() << "\nDisplay GRIB-related configuration."
                       "\n"
                       "\nUsage: "
                    << tool << " [file1 [file2 [...]]]" << std::endl;
    }

    void execute(const eckit::option::CmdArgs& /*args*/) override;
};


template <typename T>
T get(const param::MIRParametrisation& param, const std::string& name, T value) {
    param.get(name, value);
    return value;
}


void MIRConfigGrib::execute(const eckit::option::CmdArgs& args) {
    static const param::DefaultParametrisation defaults;
    const param::ConfigurationWrapper args_wrap(args);

    static const grib::Config config(LibMir::configFile(LibMir::config_file::GRIB_OUTPUT), false);

    for (const auto& arg : args) {
        Log::info() << arg << std::endl;
        std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(arg, args_wrap));

        while (input->next()) {
            std::unique_ptr<param::MIRParametrisation> param(
                new param::CombinedParametrisation(args_wrap, input->parametrisation(), defaults));
            config.find(*param);
        }
    }
}


}  // namespace mir::tools


int main(int argc, char** argv) {
    mir::tools::MIRConfigGrib tool(argc, argv);
    return tool.start();
}
