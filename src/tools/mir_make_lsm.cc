





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
#include "eckit/parser/Tokenizer.h"
#include "eckit/runtime/Context.h"

// #include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
// #include "mir/output/GribFileOutput.h"
#include "mir/data/MIRField.h"


class MIRMakeLSM : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    MIRMakeLSM(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRMakeLSM::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " [key1 key2 ...] file.grib file.lsm" << std::endl
            ;

    ::exit(1);
}

void MIRMakeLSM::run() {

    eckit::Context &ctx = eckit::Context::instance();
    const std::string &tool = ctx.runName();

    size_t argc = ctx.argc();

    if (argc <= 2) {
        usage(tool);
    }

    mir::input::GribFileInput file(ctx.argv(1));

    while (file.next()) {
        mir::input::MIRInput &input = file;

        const mir::param::MIRParametrisation &parametrisation = input.parametrisation();

        std::auto_ptr<mir::data::MIRField> field(input.field());

        std::cout << field->values(0).size() << std::endl;

        std::cout << std::endl;
    }

}


int main( int argc, char **argv ) {
    MIRMakeLSM tool(argc, argv);
    tool.start();
    return 0;
}

