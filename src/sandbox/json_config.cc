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


#include "eckit/runtime/Tool.h"
#include "mir/param/JSONConfiguration.h"


class JSONTool : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    JSONTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};


void JSONTool::run() {
    mir::param::JSONConfiguration config("/tmp/test.json");
}


int main( int argc, char **argv ) {
    JSONTool tool(argc, argv);
    tool.start();
    return 0;
}

