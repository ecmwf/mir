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


#include "eckit/filesystem/PathName.h"
#include "mir/param/MIRConfiguration.h"
#include "mir/tools/MIRTool.h"


class MIRConfig : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool);

public:

    // -- Contructors

    MIRConfig(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {}

};


void MIRConfig::usage(const std::string &tool) {
    eckit::Log::info()
            << "\n" "Usage: " << tool
            << std::endl;
}


void MIRConfig::execute(const eckit::option::CmdArgs&) {

    // const param::MIRConfiguration& conf = param::MIRConfiguration::instance();

    eckit::PathName path("~mir/etc/mir/interpolation-methods.cfg");
    eckit::Log::info() << "Path is " << path << std::endl;
    if(path.exists()) {
        eckit::Log::info() << "File exists" << std::endl;
    } else {
        eckit::Log::info() << "File does not exist" << std::endl;
        ::exit(1);
    }
}


int main(int argc, char **argv) {
    MIRConfig tool(argc, argv);
    return tool.start();
}


