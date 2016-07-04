





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

#include "mir/input/GribFileInput.h"


class MIRInspect : public eckit::Tool {

    virtual void run();

    void usage(const std::string &tool);

  public:
    MIRInspect(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRInspect::usage(const std::string &tool) {

    eckit::Log::info()
            << eckit::newl << "Usage: " << tool << " [key1 key2 ...] file.grib" << eckit::newl
            << eckit::newl << "Examples: " << eckit::newl
            << "% " << tool << " grid area file.grib" << eckit::newl
            << "% " << tool << " reduced file.grib" << eckit::newl << eckit::newl
            << "% " << tool << " regular file.grib" << eckit::newl << eckit::newl
            << "% " << tool << " truncation file.grib" << eckit::newl << eckit::newl
            << "% " << tool << " octahedral file.grib" << eckit::newl << eckit::newl

            ;

    ::exit(1);
}

void MIRInspect::run() {

    eckit::Context &ctx = eckit::Context::instance();
    const std::string &tool = ctx.runName();

    size_t argc = ctx.argc();

    if (argc <= 2) {
        usage(tool);
    }

    mir::input::GribFileInput file(ctx.argv(argc - 1));

    while (file.next()) {
        mir::input::MIRInput &input = file;

        const mir::param::MIRParametrisation &parametrisation = input.parametrisation();

        const char *sep = "";
        std::string value;

        for (size_t i = 1; i < argc - 1; i++) {
            std::cout << sep << ctx.argv(i) << "=";
            if (parametrisation.get(ctx.argv(i), value)) {
                std::cout << value;
            } else {
                std::cout << "<not found>";
            }
            sep = ", ";
        }

        std::cout << eckit::newl;
    }

}


int main( int argc, char **argv ) {
    MIRInspect tool(argc, argv);
#if (ECKIT_MAJOR_VERSION == 0) && (ECKIT_MINOR_VERSION <= 10)
    tool.start();
    return 0;
#else
    return tool.start();
#endif
}

