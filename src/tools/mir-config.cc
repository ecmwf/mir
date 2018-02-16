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


#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "mir/config/LibMir.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/FieldParametrisation.h"
#include "mir/param/Rules.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/tools/MIRTool.h"


class MIRConfig : public mir::tools::MIRTool {

    // -- Methods

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

    int minimumPositionalArguments() const {
        return 0;
    }

    void display(const mir::param::MIRParametrisation& metadata, const std::string& key) const {
        static mir::param::DefaultParametrisation defaults;
        mir::param::CombinedParametrisation combined(metadata, defaults, defaults);
        const mir::param::MIRParametrisation& c = combined;

        long paramId = 0;
        c.get("paramId", paramId);

        std::string value = "???";
        c.get(key, value);

        eckit::Log::info() << "paramId=" << paramId << "," << key << "=" << value << std::endl;
    }

public:

    // -- Contructors

    MIRConfig(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;
        options_.push_back(new SimpleOption<long>("param-id", "Display configuration with paramId"));
        options_.push_back(new SimpleOption<std::string>("rule-name", "Display configuration for named rule (default 'paramId')"));
        options_.push_back(new SimpleOption<long>("rule-value", "Display configuration with named rule value (default -1)"));
        options_.push_back(new SimpleOption<std::string>("key", "Display configuration with specific key (default 'interpolation')"));
        options_.push_back(new SimpleOption<bool>("file-rules", "Display rules as per configuration files"));
    }

};


void MIRConfig::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " [--param-id=value] [--key=key] [--file-rules] [input1.grib [input2.grib [...]]]"
            "\n" "Examples: "
            "\n" "  % " << tool << ""
            "\n" "  % " << tool << " --param-id=157"
            "\n" "  % " << tool << " --param-id=167 --key=lsm input1.grib input2.grib"
            "\n" "  % " << tool << " --rule-name=paramId --rule-value=167 --key=lsm input1.grib"
            "\n" "  % " << tool << " --file-rules"
            << std::endl;
}


void MIRConfig::execute(const eckit::option::CmdArgs& args) {

    using namespace mir::param;

    std::string key = "interpolation";
    args.get("key", key);

    for (size_t i = 0; i < args.count(); i++) {

        // Display configuration for a (specific or not) paramId and metadata from input file(s)
        mir::input::GribFileInput grib(args(i));
        while (grib.next()) {
            mir::input::MIRInput& input = grib;
            display(input.parametrisation(), key);
        }

    }

    if (!args.count()) {

        if (args.has("file-rules")) {

            // Display configuration files
            Rules fileRules;
            fileRules.readConfigurationFiles();
            eckit::Log::info() << fileRules << std::endl;

        } else  {

            // Display configuration for a paramId
            std::string ruleName("paramId");
            long ruleValue = -1;
            long paramId = 0;

            if (!args.get("param-id", paramId)) {
                args.get("rule-name", ruleName);
                args.get("rule-value", ruleValue);
                if (ruleName == "paramId") {
                    paramId = ruleValue;
                }
            }

            class DummyField : public FieldParametrisation {
                long paramId_;
                virtual void print(std::ostream&) const {}
                virtual bool get(const std::string& name, long& value) const {
                    if(name == "paramId") {
                        value = paramId_;
                        return true;
                    }
                    return FieldParametrisation::get(name, value);
                }

            public:
                DummyField(long paramId): paramId_(paramId) {}
            };

            display(DummyField(paramId), key);

        }

    }
}

int main(int argc, char **argv) {
    MIRConfig tool(argc, argv);
    return tool.start();
}

