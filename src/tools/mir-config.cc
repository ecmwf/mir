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
#include "mir/config/MIRConfiguration.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"


class MIRConfig : public mir::tools::MIRTool {

    // -- Methods

    void display(const mir::param::MIRParametrisation&, const std::string& key="");

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
        options_.push_back(new SimpleOption<eckit::PathName>("configuration", "Configuration JSON path"));
        options_.push_back(new SimpleOption<long>("param-id", "Test configuration with paramId"));
        options_.push_back(new SimpleOption<std::string>("key", "Test configuration with specific key"));
    }

};


void MIRConfig::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " [--configuration=configuration.yaml] [--param-id=value] [--key=key] [input1.grib [input2.grib [...]]]"
               "\n" "Examples: "
               "\n" "  % " << tool << ""
               "\n" "  % " << tool << " --param-id=157 input.grib"
               "\n" "  % " << tool << " --param-id=157 --configuration=test.yaml input1.grib input2.grib"
            << std::endl;
}


void MIRConfig::display(const mir::param::MIRParametrisation& parametrisation, const std::string& key) {
    if (key.length()) {
        std::string value;
        eckit::Log::info() << (parametrisation.get(key, value)? value : "<not found>") << std::endl;
    } else {
        eckit::Log::info() << parametrisation << std::endl;
    }
}


void MIRConfig::execute(const eckit::option::CmdArgs& args) {
    using namespace mir::config;
    using namespace mir::param;


    // set configuration file path
    args.get("configuration", MIRConfiguration::path);


    std::string key = "";
    args.get("key", key);


    for (size_t i = 0; i < args.count(); i++) {

        // Display configuration for a (specific or not) paramId and metadata from input file(s)
        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {
            mir::input::MIRInput& input = grib;

            const MIRParametrisation& metadata = input.parametrisation();
            long id = 0;
            args.get("param-id", id) || metadata.get("paramId", id);

            const MIRParametrisation& p(MIRConfiguration::instance().lookup(id, metadata));
            display(p, key);

        }

    }

    if (!args.count()) {
        if (args.has("param-id")) {

            // Display configuration for a paramId
            SimpleParametrisation metadata;
            long id = 0;
            args.get("param-id", id);

            const MIRParametrisation& p(MIRConfiguration::instance().lookup(id, metadata));
            display(p, key);

        } else {

            // Display configuration defaults
            display(MIRConfiguration::instance(), key);

        }
    }
}


int main(int argc, char **argv) {
    MIRConfig tool(argc, argv);
    return tool.start();
}

