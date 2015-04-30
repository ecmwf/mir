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
#include "eckit/runtime/Context.h"
#include "eckit/parser/Tokenizer.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"


class MIRTool : public eckit::Tool {

    virtual void run();

    void usage(const std::string& tool);

  public:
    MIRTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRTool::usage(const std::string& tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " [key1=value key2=value ...] input output" << std::endl
            << std::endl << "Examples: " << std::endl
            << "% " << tool << " grid=2/2 area=90/-8/12/80 input.grib output.grib" << std::endl
            << "% " << tool << " reduced=80 input.grib output.grib" << std::endl << std::endl
            << "% " << tool << " regular=80 input.grib output.grib" << std::endl << std::endl
            << "% " << tool << " truncation=63 input.grib output.grib" << std::endl << std::endl
            << "Option are:" << std::endl
            << "===========" << std::endl << std::endl

            << "   accuracy=n" << std::endl
            << "   area=north/west/south/east" << std::endl
            << "   autoresol=0/1" << std::endl
            << "   bitmap=path" << std::endl
            << "   epsilon=e" << std::endl
            << "   frame=n" << std::endl
            << "   grid=west_east/north_south" << std::endl
            << "   intermediate_gaussian=N" << std::endl
            << "   interpolation=method (e.g. bilinear)" << std::endl
            << "   nclosest=n (e.g. for k-nearest)" << std::endl
            << "   octahedral=N" << std::endl
            << "   reduced=N" << std::endl
            << "   regular=N" << std::endl
            << "   truncation=T" << std::endl

            ;

    ::exit(1);
}

void MIRTool::run() {

    eckit::Context& ctx = eckit::Context::instance();
    const std::string& tool = ctx.runName();
    size_t argc = ctx.argc();

    if (argc <= 2) {
        usage(tool);
    }

    mir::api::MIRJob job;
    mir::input::GribFileInput input(ctx.argv(argc - 2));
    mir::output::GribFileOutput output(ctx.argv(argc - 1));

    eckit::Tokenizer parse("=");
    for (size_t i = 1; i < argc - 2; i++) {
        std::vector<std::string> v;
        parse(ctx.argv(i), v);
        if (v.size() != 2) {
            usage(tool);
        }
        job.set(v[0], v[1]);
    }

    size_t i = 0;
    while (input.next()) {
        eckit::Log::info() << "FIELD: " << (++i) << std::endl;
        job.execute(input, output);
    }

}


int main( int argc, char **argv ) {
    MIRTool tool(argc, argv);
    tool.start();
    return 0;
}

