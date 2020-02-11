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


#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "mir/input/GribFileInput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/FieldParametrisation.h"
#include "mir/tools/MIRTool.h"


class MIRConfig : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const { return 0; }

    void display(const mir::param::MIRParametrisation& metadata, const std::string& key) const {
        using namespace mir ::param;

        static SimpleParametrisation empty;
        static DefaultParametrisation defaults;
        const CombinedParametrisation combined(empty, metadata, defaults);
        const MIRParametrisation& param(combined);

        long paramId = 0;
        ASSERT(metadata.get("paramId", paramId));

        std::string value = "???";
        param.get(key, value);

        eckit::Log::info() << "paramId=" << paramId << ": " << key << "=" << value << std::endl;
    }

public:
    // -- Contructors

    MIRConfig(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
        using eckit::option::SimpleOption;
        options_.push_back(new SimpleOption<long>("param-id", "Display configuration with paramId"));
        options_.push_back(new SimpleOption<std::string>("key", "Display configuration with specific key"));
    }
};


void MIRConfig::usage(const std::string& tool) const {
    eckit::Log::info() << "\n"
                          "Usage: "
                       << tool
                       << " [--key=key] [[--param-id=value]|[input1.grib [input2.grib [...]]]]"
                          "\n"
                          "Examples: "
                          "\n"
                          "  % "
                       << tool
                       << ""
                          "\n"
                          "  % "
                       << tool
                       << " --param-id=157"
                          "\n"
                          "  % "
                       << tool << " --key=lsm input1.grib input2.grib" << std::endl;
}


void MIRConfig::execute(const eckit::option::CmdArgs& args) {

    std::string key("interpolation");
    args.get("key", key);


    // Display configuration for a paramId
    long paramId = 0;
    if (args.get("param-id", paramId) || args.count() == 0) {

        class DummyField : public mir::param::FieldParametrisation {
            long paramId_;
            virtual void print(std::ostream&) const {}
            virtual bool get(const std::string& name, long& value) const {
                if (name == "paramId") {
                    value = paramId_;
                    return true;
                }
                return FieldParametrisation::get(name, value);
            }

        public:
            DummyField(long paramId) : paramId_(paramId) {}
        };

        display(DummyField(paramId), key);
    }
    else {

        for (size_t i = 0; i < args.count(); i++) {

            // Display configuration for each input file message(s)
            mir::input::GribFileInput grib(args(i));
            while (grib.next()) {
                mir::input::MIRInput& input = grib;
                display(input.parametrisation(), key);
            }
        }
    }
}

int main(int argc, char** argv) {
    MIRConfig tool(argc, argv);
    return tool.start();
}
