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

#include "mir/api/emoslib.h"


#include "eckit/runtime/Tool.h"
#include "eckit/log/Log.h"

using eckit::Log;

//----------------------------------------------------------------------------------------------------------------------

class EmosTool : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);

  public:
    EmosTool(int argc, char **argv) :  eckit::Tool(argc, argv) {
    }

};

void EmosTool::usage(const std::string &tool) {

}

static void intout(const char *name, double x, double y) {
    // extern "C" fortint intout_(const char *name,
    //                        const fortint ints[],
    //                        const fortfloat reals[],
    //                        const char *value,
    //                        const fortint name_len,
    //                        const fortint value_len);
    fortint ints[] = {0, };
    fortfloat reals[] = {x, y};
    // ASSERT(intout_(name, ints, reals, "", strlen(name), 0));
}

void EmosTool::run() {

    fortint dummy = 1;
    Log::info() << "Libemos cycle " << emosnum_(dummy) << std::endl;

    intout("grid", 2.0, 2.0);

}

//----------------------------------------------------------------------------------------------------------------------

int main( int argc, char **argv ) {
    EmosTool tool(argc, argv);
    tool.start();
    return 0;
}

