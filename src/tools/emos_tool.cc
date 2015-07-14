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
/// @date Apr 2015

#include "eckit/runtime/Tool.h"
#include "eckit/log/Log.h"

using eckit::Log;

extern "C"
{
    typedef int fortint;
    fortint emosnum_(fortint *a);
}

//----------------------------------------------------------------------------------------------------------------------

class EmosTool : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);

  public:
    EmosTool(int argc, char **argv) :  eckit::Tool(argc, argv)
    {
    }

};

void EmosTool::usage(const std::string &tool) {

}

void EmosTool::run() {

    int dummy = 1;
    Log::info() << "Libemos cycle " << (long)emosnum_(&dummy) << std::endl;

}

//----------------------------------------------------------------------------------------------------------------------

int main( int argc, char **argv ) {
    EmosTool tool(argc, argv);
    tool.start();
    return 0;
}

