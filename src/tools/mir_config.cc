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


#include "mir/param/MIRConfiguration.h"
#include "eckit/runtime/Tool.h"
#include "eckit/filesystem/PathName.h"


class MIRConfig : public eckit::Tool {

    virtual void run();


  public:
    MIRConfig(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void MIRConfig::run() {
    using namespace mir;

    const param::MIRConfiguration& conf = param::MIRConfiguration::instance();

    eckit::PathName path("~mir/etc/mir/interpolation-methods.cfg");
    eckit::Log::info() << "Path is " << path << std::endl;
    if(path.exists()) {
        eckit::Log::info() << "File exists" << std::endl;
    } else {
        eckit::Log::info() << "File does not exist" << std::endl;
        ::exit(1);
    }
}


int main( int argc, char **argv ) {
    MIRConfig tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}


