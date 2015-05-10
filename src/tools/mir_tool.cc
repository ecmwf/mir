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

#include "mir/param/MIRArgs.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GribFileInput.h"
#include "mir/output/GribFileOutput.h"


class MIRTool : public eckit::Tool {

    virtual void run();

    static void usage(const std::string& tool);

  public:
    MIRTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRTool::usage(const std::string& tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " [--key1=value --key2=value ...] input.grib output.grib" << std::endl
            << std::endl << "Examples: " << std::endl
            << "% " << tool << " --grid=2/2 --area=90/-8/12/80 input.grib output.grib" << std::endl
            << "% " << tool << " --reduced=80 input.grib output.grib" << std::endl << std::endl
            << "% " << tool << " --regular=80 input.grib output.grib" << std::endl << std::endl
            << "% " << tool << " --truncation=63 input.grib output.grib" << std::endl << std::endl
            << "Option are:" << std::endl
            << "===========" << std::endl << std::endl

            << "   --accuracy=n (number of bits per value)" << std::endl
            << "   --area=north/west/south/east" << std::endl
            << "   --autoresol=0/1" << std::endl
            << "   --bitmap=path" << std::endl
            << "   --epsilon=e (k-nearest, nearest-neighbour)" << std::endl
            << "   --frame=n" << std::endl
            << "   --grid=west_east/south_north" << std::endl
            << "   --intermediate_gaussian=N" << std::endl
            << "   --interpolation=method (e.g. bilinear)" << std::endl
            << "   --lsm.file (path to LSM to use for both input and output, in grib, only if --lsm=file)"
            << "   --lsm.file.input (path to LSM to use in input, in grib, only if --lsm=file)"
            << "   --lsm.file.output (path to LSM to use in input, in grib, only if --lsm=file)"
            << "   --lsm.interpolation.input=n (interpolation method for input lsm, default nearest-neighbour)"
            << "   --lsm.interpolation.output=n (interpolation method for output lsm, default nearest-neighbour)"
            << "   --lsm.interpolation=n (interpolation method for both lsm, default nearest-neighbour)"
            << "   --lsm.selection (selection method for both input and output lsm, e.g. auto or file)"
            << "   --lsm.selection.input (selction method for input lsm, in grib, e.g. auto or file)"
            << "   --lsm.selection.output (selction method for output lsm, in grib, e.g. auto or file)"
            << "   --lsm=0/1 (use lsm when interpolating)"
            << "   --nclosest=n (e.g. for k-nearest)" << std::endl
            << "   --octahedral=N" << std::endl
            << "   --packing=p (e.g. second-order)" << std::endl
            << "   --reduced=N" << std::endl
            << "   --regular=N" << std::endl
            << "   --rotation=lat/lon" << std::endl
            << "   --truncation=T" << std::endl

            ;

    ::exit(1);
}

void MIRTool::run() {

    mir::param::MIRArgs args(&usage, 2);

    mir::input::GribFileInput input(args.args(0));
    mir::output::GribFileOutput output(args.args(1));

    mir::api::MIRJob job;
    args.copyValuesTo(job);


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

