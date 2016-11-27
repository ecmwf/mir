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

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

public:

    // -- Contructors

    MIRConfig(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;
        options_.push_back(new SimpleOption<eckit::PathName>("configuration", "Configuration JSON path"));
        options_.push_back(new SimpleOption<eckit::PathName>("metadata", "Test configuration with metadata from file"));
        options_.push_back(new SimpleOption<long>("param-id", "Test configuration with paramId"));
    }

};


void MIRConfig::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool
            << std::endl;
}


void MIRConfig::execute(const eckit::option::CmdArgs& args) {
    using mir::config::MIRConfiguration;
    using mir::param::MIRParametrisation;


    // get options
    std::string path_config = "";
    std::string path_metadata = "";
    long param_id = 0;
    args.get("configuration", path_config);
    args.get("metadata", path_metadata);
    args.get("param-id", param_id);


    // configure with file (or use internal defaults)
    MIRConfiguration& config = MIRConfiguration::instance();
    path_config.length()? config.configure(path_config)
                        : config.configure();

    if (path_metadata.length()) {

        // Display configuration for a (specific or not) paramId and metadata
        mir::input::GribFileInput file(argv(argc() - 1));
        while (file.next()) {
            mir::input::MIRInput& input = file;

            const MIRParametrisation& metadata = input.parametrisation();
            long id = param_id? param_id: metadata.get("paramId", id);

            eckit::ScopedPtr< const MIRParametrisation > p(config.lookup(id, metadata));
            eckit::Log::info() << "MIRConfiguration::lookup(" << id << ", metadata): " << *p << std::endl;

        }

    } else if (param_id) {

        // Display configuration for a paramId
        const MIRParametrisation& metadata = mir::param::SimpleParametrisation();

        eckit::ScopedPtr< const MIRParametrisation > p(config.lookup(param_id, metadata));
        eckit::Log::info() << "MIRConfiguration::lookup(" << param_id << ", empty): " << *p << std::endl;

    } else {

        // Display configuration defaults
        eckit::ScopedPtr< const MIRParametrisation > p(config.defaults());
        eckit::Log::info() << "MIRConfiguration::defaults(): " << *p << std::endl;

    }
}


int main(int argc, char **argv) {
    MIRConfig tool(argc, argv);
    return tool.start();
}

