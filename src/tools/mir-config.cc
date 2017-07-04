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
#include "mir/input/GribFileInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"


class MIRConfig : public mir::tools::MIRTool {

    // -- Methods

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

    int minimumPositionalArguments() const {
        return 0;
    }

public:

    // -- Contructors

    MIRConfig(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;
        options_.push_back(new SimpleOption<long>("param-id", "Test configuration with paramId"));
        options_.push_back(new SimpleOption<std::string>("key", "Test configuration with specific key"));
    }

};


void MIRConfig::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " [--param-id=value] [--key=key] [input1.grib [input2.grib [...]]]"
            "\n" "Examples: "
            "\n" "  % " << tool << ""
            "\n" "  % " << tool << " --param-id=157"
            "\n" "  % " << tool << " --param-id=167 --key=lsm input1.grib input2.grib"
            << std::endl;
}


void MIRConfig::execute(const eckit::option::CmdArgs& args) {

    using namespace mir::param;
    const DefaultParametrisation defaults;

    std::string key = "interpolation";
    args.get("key", key);

    for (size_t i = 0; i < args.count(); i++) {

        // Display configuration for a (specific or not) paramId and metadata from input file(s)
        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {
            mir::input::MIRInput& input = grib;

            const MIRParametrisation& metadata = input.parametrisation();
            MIRCombinedParametrisation combined(metadata, defaults, defaults);
            const MIRParametrisation& c = combined;

            long paramId = 0;
            c.get("paramId", paramId);

            std::string value = "???";
            c.get(key, value);

            std::cout << "paramId=" << paramId << "," << key << "=" << value << std::endl;

        }

    }

    // if (!args.count()) {
    //     if (args.has("param-id")) {

    //         // Display configuration for a paramId
    //         SimpleParametrisation metadata;
    //         long id = 0;
    //         args.get("param-id", id);

    //         const MIRParametrisation& p(config.pick(id, metadata));
    //         display(p, key);

    // }

}


int main(int argc, char **argv) {
    MIRConfig tool(argc, argv);
    return tool.start();
}

