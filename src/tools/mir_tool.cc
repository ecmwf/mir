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




#include "eckit/log/Plural.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/runtime/Tool.h"

#include "experimental/eckit/la/LinearAlgebra.h"

#include "mir/api/MIRJob.h"
#include "mir/input/DummyInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VODInput.h"
#include "mir/input/WindInput.h"
#include "mir/lsm/LSMChooser.h"
#include "mir/method/Method.h"
#include "mir/output/GribFileOutput.h"
#include "mir/output/UVOutput.h"
#include "mir/output/WindOutput.h"
#include "mir/packing/Packer.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/option/FactoryOption.h"
#include "mir/param/option/Separator.h"
#include "mir/param/option/SimpleOption.h"
#include "mir/param/option/VectorOption.h"


using mir::param::option::Option;
using mir::param::option::SimpleOption;
using mir::param::option::Separator;
using mir::param::option::VectorOption;
using mir::param::option::FactoryOption;

class MIRTool : public eckit::Tool {

    virtual void run();
    void process(mir::api::MIRJob &, mir::input::MIRInput &, mir::output::MIROutput &, const std::string &);

    static void usage(const std::string &tool);

  public:
    MIRTool(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }

};

void MIRTool::usage(const std::string &tool) {

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



    std::vector<const Option *> options;


    //==============================================
    options.push_back(new Separator("Transform"));
    options.push_back(new SimpleOption<bool>("autoresol", "Turn on automatic truncation"));
    options.push_back(new SimpleOption<size_t>("truncation", "Truncation input field"));
    options.push_back(new SimpleOption<bool>("vod2uv", "Input is Vorticity and Divergence, convertion to U/V requested"));

    //==============================================
    options.push_back(new Separator("Interpolation"));
    options.push_back(new VectorOption<double>("grid", "Interpolate to the regular grid: west_east/south_north", 2));
    options.push_back(new SimpleOption<size_t>("regular", "Interpolate to the regular gaussian grid N"));
    options.push_back(new SimpleOption<size_t>("reduced", "Interpolate to the regular gaussian grid N (pre 2016)"));
    options.push_back(new SimpleOption<size_t>("octahedral", "Interpolate to the regular gaussian grid N"));
    options.push_back(new SimpleOption<bool>("wind", "Use vector interpolation for wind (not yet)"));


    //==============================================
    options.push_back(new Separator("Methods"));
    options.push_back(new FactoryOption<mir::method::MethodFactory>("interpolation", "Grid to grid interpolation method"));
    options.push_back(new SimpleOption<size_t>("intermediate_gaussian", "Transform from SH to this gaussian number first"));
    options.push_back(new SimpleOption<double>("epsilon", "Used by methods k-nearest and nearest-neighbour"));
    options.push_back(new SimpleOption<size_t>("nclosest", "Used by methods k-nearest"));
    options.push_back(new SimpleOption<bool>("caching", "Caching of weights and grids (default 1)"));
    options.push_back(new FactoryOption<eckit::la::LinearAlgebra>("backend", "Linear algebra backend (default 'generic')"));

    //==============================================
    options.push_back(new Separator("Rotation"));
    options.push_back(new VectorOption<double>("rotation", "Rotate the grid by moving the south pole to: latitude/longitude", 2));

    //==============================================
    options.push_back(new Separator("Filtering"));
    options.push_back(new VectorOption<double>("area", "Specify the cropping area: north/west/south/east", 4));
    options.push_back(new SimpleOption<eckit::PathName>("bitmap", "Path to the bitmap to apply"));
    options.push_back(new SimpleOption<size_t>("frame", "Size of the frame"));

    //==============================================
    options.push_back(new Separator("Land sea mask management"));
    options.push_back(new SimpleOption<bool>("lsm", "Use land sea mask (lsm) when interpolating grid to grid"));

    options.push_back(new FactoryOption<mir::method::MethodFactory>("lsm.interpolation", "Interpolation method for both input and output lsm, default nearest-neighbour"));
    options.push_back(new FactoryOption<mir::lsm::LSMChooser>("lsm.selection", "Selection method for both input and output lsm"));
    options.push_back(new SimpleOption<eckit::PathName>("lsm.file", "Path to lsm to use for both input and output, in grib, only if --lsm.selection=file"));

    options.push_back(new FactoryOption<mir::method::MethodFactory>("lsm.interpolation.input", "Interpolation method for lsm, default nearest-neighbour"));
    options.push_back(new FactoryOption<mir::lsm::LSMChooser>("lsm.selection.input", "Selection method for input lsm"));
    options.push_back(new SimpleOption<eckit::PathName>("lsm.file.input", "Path to lsm to use for input lsm, in grib, only if --lsm.selection=file"));

    options.push_back(new FactoryOption<mir::method::MethodFactory>("lsm.interpolation.output", "Interpolation method for lsm, default nearest-neighbour"));
    options.push_back(new FactoryOption<mir::lsm::LSMChooser>("lsm.selection.output", "Selection method for output lsm"));
    options.push_back(new SimpleOption<eckit::PathName>("lsm.file.output", "Path to lsm to use for output lsm, in grib, only if --lsm.selection=file"));

    options.push_back(new SimpleOption<double>("lsm.weight.adjustment", "Weight adjustment factor when applying LSM (default 0.2)"));
    options.push_back(new SimpleOption<double>("lsm.value.threshold", "Value threshold when convering LSM field to mask (default 0.5)"));

    //==============================================
    options.push_back(new Separator("Unstructured grids support"));
    options.push_back(new SimpleOption<eckit::PathName>("latitudes", "Path GRIB file of latitudes"));
    options.push_back(new SimpleOption<eckit::PathName>("longitudes", "Path GRIB file of longitudes"));


    //==============================================
    options.push_back(new Separator("GRIB Output"));
    options.push_back(new SimpleOption<size_t>("accuracy", "Number of bits per value"));
    options.push_back(new FactoryOption<mir::packing::Packer>("packing", "GRIB packing method"));

    //==============================================
    options.push_back(new Separator("Miscellaneous"));
    options.push_back(new SimpleOption<std::string>("style", "Compatibility with style=dissemination (ignored)"));

    //==============================================
    options.push_back(new Separator("Debugging"));
    options.push_back(new SimpleOption<bool>("dummy", "Use dummy data"));
    options.push_back(new SimpleOption<bool>("checkerboard", "Create checkerboard field"));


    // {"", 0, "GRIB Output"},
    // {"accuracy", "n", "number of bits per value",},
    // {"packing", "p", "e.g. second-order",},


    mir::param::MIRArgs args(&usage, 2, options);

    // If we want to control the backend in MARS/PRODGEN, we can move that to MIRJob
    std::string backend;
    if(args.get("backend", backend)) {
        eckit::la::LinearAlgebra::backend(backend);
    }

    mir::api::MIRJob job;
    args.copyValuesTo(job);

    bool wind = false;
    bool vod2uv = false;
    bool dummy = false;

    args.get("wind", wind);
    args.get("vod2uv", vod2uv);
    args.get("dummy", dummy);


    if (dummy) {
        mir::input::DummyInput input;
        mir::output::GribFileOutput output(args.args(1));
        process(job, input, output, "field");
    } else if (wind) {
        ASSERT(!vod2uv);
        ASSERT(!args.has("latitudes") &&  !args.has("longitudes"));

        mir::input::GribFileInput input1(args.args(0), 0, 2);
        mir::input::GribFileInput input2(args.args(0), 1, 2);

        mir::output::GribFileOutput output(args.args(1));


        mir::input::WindInput winput(input1, input2);
        mir::output::WindOutput woutput(output, output);
        process(job, winput, woutput, "wind");

    } else if (vod2uv) {
        ASSERT(!wind);
        ASSERT(!args.has("latitudes") &&  !args.has("longitudes"));

        mir::input::GribFileInput input1(args.args(0), 0, 2);
        mir::input::GribFileInput input2(args.args(0), 1, 2);
        mir::output::GribFileOutput output(args.args(1));

        mir::input::VODInput winput(input1, input2);
        mir::output::UVOutput woutput(output, output);
        process(job, winput, woutput, "wind");

    } else {

        mir::input::GribFileInput input(args.args(0));
        mir::output::GribFileOutput output(args.args(1));

        std::string path_lat, path_lon;
        ASSERT(args.has("latitudes") ==  args.has("longitudes"));
        if (args.get("latitudes", path_lat) &&  args.get("longitudes", path_lon)) {
            input.setAuxilaryFiles(path_lat, path_lon);
        }

        process(job, input, output, "field");
    }
}

void MIRTool::process(mir::api::MIRJob &job, mir::input::MIRInput &input, mir::output::MIROutput &output, const std::string &what) {
    eckit::Timer timer("Total time");

    size_t i = 0;
    while (input.next()) {
        eckit::Log::info() << "============> " << what << ": " << (++i) << std::endl;
        job.execute(input, output);
    }

    eckit::Log::info() << eckit::Plural(i, what) << " in " << eckit::Seconds(timer.elapsed()) <<
                       ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;

}


int main( int argc, char **argv ) {
    MIRTool tool(argc, argv);
    tool.start();
    return 0;
}

