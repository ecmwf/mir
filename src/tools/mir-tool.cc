/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"
#include "eckit/log/ResourceUsage.h"
#include "mir/action/plan/Executor.h"
#include "mir/api/MIRJob.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/caching/interpolator/InterpolatorLoader.h"
#include "mir/config/LibMir.h"
#include "mir/input/DummyInput.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"
#include "mir/lsm/LSMChooser.h"
#include "mir/method/Method.h"
#include "mir/mir_ecbuild_config.h"
#include "mir/output/GeoPointsFileOutput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/packing/Packer.h"
#include "mir/style/MIRStyle.h"
#include "mir/tools/MIRTool.h"


class MIRToolConcrete : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 2;
    }

    void process(mir::api::MIRJob&, mir::input::MIRInput&, mir::output::MIROutput&, const std::string&);

public:

    MIRToolConcrete(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        //==============================================
        options_.push_back(new Separator("Transform"));
        options_.push_back(new SimpleOption<bool>("autoresol", "Turn on automatic truncation"));
        options_.push_back(new SimpleOption<size_t>("truncation", "Truncation input field"));
        options_.push_back(new SimpleOption<bool>("vod2uv", "Input is Vorticity and Divergence, conversion to u/v or U/V requested"));

        //==============================================
        options_.push_back(new Separator("Interpolation"));
        options_.push_back(new VectorOption<double>("grid", "Interpolate to the regular grid: west_east/south_north", 2));
        options_.push_back(new SimpleOption<size_t>("regular", "Interpolate to the regular gaussian grid N"));
        options_.push_back(new SimpleOption<size_t>("reduced", "Interpolate to the regular gaussian grid N (pre 2016)"));
        options_.push_back(new SimpleOption<size_t>("octahedral", "Interpolate to the regular gaussian grid N"));
        options_.push_back(new SimpleOption<std::string>("gridname", "Interpolate to given grid name"));
        options_.push_back(new SimpleOption<bool>("area-defines-grid", "Bounding box defines grid (only applicable to regular latitude/longitude grids"));

        options_.push_back(new SimpleOption<bool>("wind", "Use vector interpolation for wind (not yet)"));
        options_.push_back(new SimpleOption<eckit::PathName>("same", "Interpolate to the same grid as the one provided in the first GRIB of the grib file"));
        options_.push_back(new SimpleOption<eckit::PathName>("griddef", "File containing a list of lat/lon pairs"));

        //==============================================
        options_.push_back(new Separator("Methods"));
        options_.push_back(new FactoryOption<mir::method::MethodFactory>("interpolation", "Grid to grid interpolation method"));
        options_.push_back(new SimpleOption<size_t>("intermediate_gaussian", "Transform from SH to this gaussian number first"));
        options_.push_back(new SimpleOption<size_t>("nclosest", "Used by methods k-nearest"));
        options_.push_back(new SimpleOption<bool>("caching", "Caching of weights and grids (default 1)"));
        options_.push_back(new FactoryOption<eckit::linalg::LinearAlgebra>("backend", "Linear algebra backend (default '" + eckit::linalg::LinearAlgebra::backend().name() + "')"));

        //==============================================
        options_.push_back(new Separator("Rotation"));
        options_.push_back(new VectorOption<double>("rotation", "Rotate the grid by moving the south pole to: latitude/longitude", 2));

        //==============================================
        options_.push_back(new Separator("Filtering"));
        options_.push_back(new VectorOption<double>("area", "Specify the cropping area: north/west/south/east", 4));
        options_.push_back(new SimpleOption<eckit::PathName>("bitmap", "Path to the bitmap to apply"));
        options_.push_back(new SimpleOption<size_t>("frame", "Size of the frame"));

        //==============================================
        options_.push_back(new Separator("Compute"));
        options_.push_back(new SimpleOption<std::string>("formula", "Formula to apply on field"));

        //==============================================
        options_.push_back(new Separator("Land-sea mask management"));
        options_.push_back(new SimpleOption<bool>("lsm", "Use land-sea mask (lsm) when interpolating grid to grid"));

        options_.push_back(new FactoryOption<mir::method::MethodFactory>("lsm-interpolation", "Interpolation method for both input and output lsm, default nearest-neighbour"));
        options_.push_back(new FactoryOption<mir::lsm::LSMChooser>("lsm-selection", "Selection method for both input and output lsm"));
        options_.push_back(new SimpleOption<eckit::PathName>("lsm-file", "Path to lsm to use for both input and output, in grib, only if --lsm-selection=file"));

        options_.push_back(new FactoryOption<mir::method::MethodFactory>("lsm-interpolation-input", "Interpolation method for lsm, default nearest-neighbour"));
        options_.push_back(new FactoryOption<mir::lsm::LSMChooser>("lsm-selection-input", "Selection method for input lsm"));
        options_.push_back(new SimpleOption<eckit::PathName>("lsm-file-input", "Path to lsm to use for input lsm, in grib, only if --lsm-selection=file"));

        options_.push_back(new FactoryOption<mir::method::MethodFactory>("lsm-interpolation-output", "Interpolation method for lsm, default nearest-neighbour"));
        options_.push_back(new FactoryOption<mir::lsm::LSMChooser>("lsm-selection-output", "Selection method for output lsm"));
        options_.push_back(new SimpleOption<eckit::PathName>("lsm-file-output", "Path to lsm to use for output lsm, in grib, only if --lsm-selection=file"));

        options_.push_back(new SimpleOption<double>("lsm-weight-adjustment", "Weight adjustment factor when applying LSM (default 0.2)"));
        options_.push_back(new SimpleOption<double>("lsm-value-threshold", "Value threshold when converting LSM field to mask (default 0.5)"));

        //==============================================
        options_.push_back(new Separator("Unstructured grids support"));
        options_.push_back(new SimpleOption<bool>("geopoints", "Input is a geopoints file"));
        options_.push_back(new SimpleOption<eckit::PathName>("latitudes", "Path GRIB file of latitudes"));
        options_.push_back(new SimpleOption<eckit::PathName>("longitudes", "Path GRIB file of longitudes"));

        //==============================================
        options_.push_back(new Separator("Configuration"));
        options_.push_back(new SimpleOption<eckit::PathName>("configuration", "Configuration JSON path (default ~mir/etc/mir/configuration.json)"));

        //==============================================
        options_.push_back(new Separator("GRIB Output"));
        options_.push_back(new SimpleOption<size_t>("accuracy", "Number of bits per value"));
        options_.push_back(new FactoryOption<mir::packing::Packer>("packing", "GRIB packing method"));
        options_.push_back(new SimpleOption<size_t>("edition", "GRIB edition number"));
        options_.push_back(new SimpleOption<bool>("remove-local-extension", "Remove GRIB local extension"));

        //==============================================
        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(new FactoryOption<mir::style::MIRStyleFactory>("style", "Select how the interpolations are performed"));
        options_.push_back(new FactoryOption<mir::caching::legendre::LegendreLoaderFactory>("legendre-loader", "Select the scheme to load coefficients"));
        options_.push_back(new FactoryOption<mir::caching::interpolator::InterpolatorLoaderFactory>("interpolator-loader", "Select the scheme to load interpolation weights"));
        options_.push_back(new FactoryOption<mir::action::Executor>("executor", "Select whether threads are used on not"));
        options_.push_back(new SimpleOption<long>("trans-fast-legendre-transform", "Trans Fast Legendre Transform method"));

        //==============================================
        options_.push_back(new Separator("Debugging"));
        options_.push_back(new SimpleOption<bool>("dummy", "Use dummy data"));
        options_.push_back(new SimpleOption<bool>("checkerboard", "Create checkerboard field"));
        options_.push_back(new SimpleOption<bool>("pattern", "Create reference pattern field"));
        options_.push_back(new SimpleOption<size_t>("param-id", "Set parameter id"));
        options_.push_back(new SimpleOption<bool>("0-1", "Set pattern and checkerboard values between 0 and 1"));
        options_.push_back(new VectorOption<long>("frequencies", "Set pattern and checkerboard frequencies", 2));
    }

};


void MIRToolConcrete::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " [--key1=value [--key2=value [...]]] input.grib output.grib"
            "\n" "Examples: "
            "\n" "  % " << tool << " --grid=2/2 --area=90/-8/12/80 input.grib output.grib"
            "\n" "  % " << tool << " --reduced=80 input.grib output.grib"
            "\n" "  % " << tool << " --regular=80 input.grib output.grib"
            "\n" "  % " << tool << " --truncation=63 input.grib output.grib"
            << std::endl;
}


void MIRToolConcrete::execute(const eckit::option::CmdArgs& args) {

    eckit::ResourceUsage usage("mir-tool");

    // {"", 0, "GRIB Output"},
    // {"accuracy", "n", "number of bits per value",},
    // {"packing", "p", "e.g. second-order",},


    // If we want to control the backend in MARS/PRODGEN, we can move that to MIRJob
    std::string backend;
    if (args.get("backend", backend)) {
        eckit::linalg::LinearAlgebra::backend(backend);
    }

    mir::api::MIRJob job;
    args.configure(job);

    std::string same;
    if (args.get("same", same)) {
        mir::input::GribFileInput input(same);
        ASSERT(input.next());
        job.representationFrom(input);
    }

    bool wind = false;
    bool vod2uv = false;
    bool dummy = false;

    args.get("wind", wind);
    args.get("vod2uv", vod2uv);
    args.get("dummy", dummy);



    if (dummy) {
        mir::input::DummyInput input;
        mir::output::GribFileOutput output(args(1));
        process(job, input, output, "field");
        return;
    }

    if (wind) {
        ASSERT(!vod2uv);
        ASSERT(!args.has("latitudes") &&  !args.has("longitudes"));

        mir::input::GribFileInput input1(args(0), 0, 2);
        mir::input::GribFileInput input2(args(0), 1, 2);

        mir::output::GribFileOutput output(args(1));


        mir::input::VectorInput input(input1, input2);
        process(job, input, output, "wind");
        return;

    }

    if (vod2uv) {
        ASSERT(!wind);
        ASSERT(!args.has("latitudes") &&  !args.has("longitudes"));

        mir::input::GribFileInput vort_input(args(0), 0, 2);
        mir::input::GribFileInput div_input(args(0), 1, 2);
        mir::output::GribFileOutput output(args(1));

        mir::input::VectorInput input(vort_input, div_input);
        process(job, input, output, "wind");
        return;

    }

    if (args.has("geopoints")) {
        mir::input::GeoPointsFileInput input(args(0));
        mir::output::GribFileOutput output(args(1));
        process(job, input, output, "field");
        return;
    }

    std::string griddef;
    if (args.has("griddef")) {
        mir::input::GribFileInput input(args(0));
        mir::output::GeoPointsFileOutput output(args(1));
        process(job, input, output, "field");
        return;
    }

    mir::input::GribFileInput input(args(0));
    mir::output::GribFileOutput output(args(1));

    std::string path_lat, path_lon;
    ASSERT(args.has("latitudes") ==  args.has("longitudes"));
    if (args.get("latitudes", path_lat) &&  args.get("longitudes", path_lon)) {
        input.setAuxilaryFiles(path_lat, path_lon);
    }

    process(job, input, output, "field");
}


void MIRToolConcrete::process(mir::api::MIRJob &job, mir::input::MIRInput &input, mir::output::MIROutput &output, const std::string &what) {
    eckit::Timer timer("Total time");

    eckit::Log::debug() << "Using '" << eckit::linalg::LinearAlgebra::backend().name() << "' backend." << std::endl;

    size_t i = 0;
    while (input.next()) {
        eckit::Log::debug<mir::LibMir>() << "============> " << what << ": " << (++i) << std::endl;
        job.execute(input, output);
    }

    eckit::Log::info() << eckit::Plural(i, what) << " in " << eckit::Seconds(timer.elapsed()) <<
                       ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;
}


int main(int argc, char **argv) {
    MIRToolConcrete tool(argc, argv);
    return tool.start();
}

