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

#include "eckit/log/Timer.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Plural.h"


static mir::param::ArgOptions options[] = {

    {"", 0, "Transform"},
    {"autoresol", "0/1",},
    {"truncation", "T",},

    {"", 0, "Interpolation"},
    {"grid", "west_east/south_north",},



    {"regular", "N",},
    {"reduced", "N",},
    {"octahedral", "N",},

    {"", 0, "Methods"},
    {"interpolation", "method", "e.g. bilinear",},
    {"intermediate_gaussian", "N",},
    {"epsilon", "e", "k-nearest, nearest-neighbour",},
    {"nclosest", "n", "k-nearest",},


    {"", 0, "Rotation"},
    {"rotation", "lat/lon",},

    {"", 0, "Filtering"},
    {"area", "north/west/south/east",},
    {"bitmap", "path",},
    {"frame", "n",},

    {"", 0, "Land sea mask management"},


    {"lsm", "0/1", "use lsm when interpolating",},
    {"lsm.interpolation", "name", "interpolation method for both lsm, default nearest-neighbour",},
    {"lsm.selection", "name", "selection method for both input and output lsm, e.g. auto or file",},
    {"lsm.file", "path", "path to LSM to use for both input and output, in grib, only if --lsm file",},

    {"lsm.file.input", "path", "path to LSM to use in input, in grib, only if --lsm file",},
    {"lsm.file.output", "path", "path to LSM to use in input, in grib, only if --lsm file",},
    {"lsm.interpolation.input", "name", "interpolation method for input lsm, default nearest-neighbour",},
    {"lsm.interpolation.output", "name", "interpolation method for output lsm, default nearest-neighbour",},
    {"lsm.selection.input", "name", "selction method for input lsm, in grib, e.g. auto or file",},
    {"lsm.selection.output", "name", "selction method for output lsm, in grib, e.g. auto or file",},
    {"", 0, "GRIB Output"},
    {"accuracy", "n", "number of bits per value",},
    {"packing", "p", "e.g. second-order",},





    {0, },
};

class MIRTool : public eckit::Tool {

    virtual void run();

    static void usage(const std::string & tool);

  public:
    MIRTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRTool::usage(const std::string & tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " [--key1=value --key2=value ...] input.grib output.grib" << std::endl
            << std::endl << "Examples: " << std::endl
            << "  % " << tool << " --grid=2/2 --area=90/-8/12/80 input.grib output.grib" << std::endl
            << "  % " << tool << " --reduced=80 input.grib output.grib" << std::endl
            << "  % " << tool << " --regular=80 input.grib output.grib" << std::endl
            << "  % " << tool << " --truncation=63 input.grib output.grib" << std::endl
            ;
}

void MIRTool::run() {

    eckit::Timer timer("Total time");


    mir::param::MIRArgs args(&usage, 2, options);

    mir::input::GribFileInput input(args.args(0));
    mir::output::GribFileOutput output(args.args(1));

    mir::api::MIRJob job;
    args.copyValuesTo(job);


    size_t i = 0;
    while (input.next()) {
        eckit::Log::info() << "FIELD: " << (++i) << std::endl;
        job.execute(input, output);
    }

    eckit::Log::info() << eckit::Plural(i, "field") << " in " << eckit::Seconds(timer.elapsed()) <<
                       ", rate: " << double(i) / double(timer.elapsed()) << " fields/s" << std::endl;

}


int main( int argc, char **argv ) {
    MIRTool tool(argc, argv);
    tool.start();
    return 0;
}

