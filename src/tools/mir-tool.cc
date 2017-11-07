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
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/plan/Executor.h"
#include "mir/api/MIRJob.h"
#include "mir/caching/interpolator/InterpolatorLoader.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/config/LibMir.h"
#include "mir/input/DummyInput.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"
#include "mir/lsm/LSMSelection.h"
#include "mir/method/Method.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/mir_ecbuild_config.h"
#include "mir/output/GeoPointsFileOutput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/packing/Packer.h"
#include "mir/style/IntermediateGrid.h"
#include "mir/style/MIRStyle.h"
#include "mir/style/SpectralOrder.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/PointSearch.h"
#include "mir/util/option/VectorOfUnknownLengthOption.h"


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
        using mir::util::option::VectorOfUnknownLengthOption;

        //==============================================
        options_.push_back(new Separator("Spectral transforms"));
        options_.push_back(new SimpleOption<bool>("autoresol", "Control automatic truncation"));
        options_.push_back(new FactoryOption<mir::style::SpectralOrderFactory>("spectral-order", "Spectral/gridded transform order of accuracy)"));
        options_.push_back(new FactoryOption<mir::style::IntermediateGridFactory>("spectral-intermediate-grid", "Spectral/gridded transform associated grid type or name"));
        options_.push_back(new SimpleOption<size_t>("truncation", "Spectral truncation"));
        options_.push_back(new SimpleOption<bool>("vod2uv", "Input is vorticity and divergence (vo/d), convert to Cartesian components (u/v or U/V)"));

        //==============================================
        options_.push_back(new Separator("Interpolation"));
        options_.push_back(new VectorOption<double>("grid", "Interpolate to a regular latitude/longitude grid (regular_ll), provided the West-East & South-North increments", 2));
        options_.push_back(new SimpleOption<size_t>("regular", "Interpolate to the regular Gaussian grid N (regular_gg), with N the number of parallels between pole and equator (N>=2)"));
        options_.push_back(new SimpleOption<size_t>("reduced", "Interpolate to the reduced Gaussian grid N (reduced_gg), with N the number of parallels between pole and equator (pre-defined list of N values.) N must be 16, 24, 32, 48, 64, 80, 96, 128, 160, 200, 256, 320, 400, 512, 576, 640 or 800"));
        options_.push_back(new SimpleOption<size_t>("octahedral", "Interpolate to the octahedral reduced Gaussian grid N (reduced_gg), with N the number of parallels between pole and equator (N>=2)"));
        options_.push_back(new SimpleOption<std::string>("gridname", "Interpolate to given grid name"));
        options_.push_back(new VectorOption<double>("rotation", "Rotate the grid by moving the South pole to latitude/longitude", 2));

        options_.push_back(new SimpleOption<bool>("wind", "Control vector interpolation for wind (not yet)"));
        options_.push_back(new SimpleOption<eckit::PathName>("same", "Interpolate to the same grid type as the first GRIB message in file"));
        options_.push_back(new SimpleOption<eckit::PathName>("griddef", "Path to GRIB file containing a list of latitude/longitude pairs"));

        options_.push_back(new FactoryOption<mir::method::MethodFactory>("interpolation", "Grid to grid interpolation method"));
        options_.push_back(new SimpleOption<size_t>("nclosest", "Number of points neighbours to weight (k), used by methods k-nearest"));
        options_.push_back(new FactoryOption<mir::method::knn::distance::DistanceWeightingFactory>("distance-weighting", "Distance weighting method, used by methods k-nearest"));
        options_.push_back(new SimpleOption<bool>("caching", "Caching of weights and grids (default 1)"));
        options_.push_back(new FactoryOption<eckit::linalg::LinearAlgebra>("backend", "Linear algebra backend (default '" + eckit::linalg::LinearAlgebra::backend().name() + "')"));
        options_.push_back(new SimpleOption<std::string>("input-mesh-generator", "Input mesh generator"));
        options_.push_back(new SimpleOption<std::string>("input-mesh-file", "Input mesh file (default <empty>)"));
        options_.push_back(new SimpleOption<std::string>("output-mesh-generator", "Output mesh generator"));
        options_.push_back(new SimpleOption<std::string>("output-mesh-file", "Output mesh file (default <empty>)"));

        options_.push_back(new FactoryOption<mir::util::PointSearchTreeFactory>("point-search-trees", "Control memory management of k-d trees"));

        //==============================================
        options_.push_back(new Separator("Filtering"));
        options_.push_back(new VectorOption<double>("area", "Specify the cropping area: north/west/south/east", 4));
        options_.push_back(new SimpleOption<eckit::PathName>("bitmap", "Path to the bitmap to apply"));
        options_.push_back(new SimpleOption<size_t>("frame", "Size of the frame"));
        options_.push_back(new SimpleOption<bool>("globalise", "Make the field global, adding missing values if needed"));
        options_.push_back(new SimpleOption<std::string>("globalise-gridname", "Unstructured grid globalise using gridname (default O16)"));
        options_.push_back(new SimpleOption<std::string>("globalise-missing-radius", "Unstructured grid globalise minimum distance to insert missing values if needed (default 555975. [m])"));

        //==============================================
        options_.push_back(new Separator("Compute"));
        options_.push_back(new SimpleOption<std::string>("formula", "Formula to apply on field"));

        //==============================================
        options_.push_back(new Separator("Land-sea mask handling"));
        options_.push_back(new SimpleOption<bool>("lsm", "Use land-sea mask (lsm) when interpolating grid to grid"));
        options_.push_back(new VectorOfUnknownLengthOption<long>("lsm-parameter-list", "Use land-sea mask (lsm) only for specific parameters"));

        for (const std::string& io : {"", "input", "output"}) {
            const std::string which = io.length()? io : "both input and output";
            const std::string key = (io.length()? "-" : "") + io;
            options_.push_back(new FactoryOption<mir::method::MethodFactory>("lsm-interpolation" + key, "LSM interpolation method for " + which + ", default nearest-neighbour"));
            options_.push_back(new FactoryOption<mir::lsm::LSMSelection>("lsm-selection" + key, "LSM selection method for " + which));
            options_.push_back(new SimpleOption<std::string>("lsm-named" + key, "If --lsm-selection" + key + "=named, LSM name to use for " + which));
            options_.push_back(new SimpleOption<eckit::PathName>("lsm-file" + key, "If --lsm-selection" + key + "=file, LSM grib file path to use for " + which));
            options_.push_back(new SimpleOption<double>("lsm-value-threshold" + key, "If --lsm-selection" + key + "=file, LSM field greater-or-equal to value threshold, when converting to mask for " + which + " (default 0.5)"));
        }
        options_.push_back(new SimpleOption<double>("lsm-weight-adjustment", "LSM interpolation weight adjustment factor (default 0.2)"));

        //==============================================
        options_.push_back(new Separator("Unstructured grids support"));
        options_.push_back(new SimpleOption<bool>("geopoints", "Input is a geopoints file"));
        options_.push_back(new SimpleOption<eckit::PathName>("latitudes", "Path GRIB file of latitudes"));
        options_.push_back(new SimpleOption<eckit::PathName>("longitudes", "Path GRIB file of longitudes"));

        //==============================================
        options_.push_back(new Separator("GRIB Output"));
        options_.push_back(new SimpleOption<size_t>("accuracy", "Number of bits per value"));
        options_.push_back(new FactoryOption<mir::packing::Packer>("packing", "GRIB packing method"));
        options_.push_back(new SimpleOption<size_t>("edition", "GRIB edition number"));
        options_.push_back(new SimpleOption<bool>("remove-local-extension", "Remove GRIB local extension"));
        options_.push_back(new SimpleOption<std::string>("metadata", "Set eccodes keys to integer values (a=b,c=d,..)"));

        //==============================================
        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(new FactoryOption<mir::style::MIRStyleFactory>("style", "Select how the interpolations are performed"));
        options_.push_back(new FactoryOption<mir::action::Executor>("executor", "Select whether threads are used or not"));
        options_.push_back(new SimpleOption<long>("trans-fast-legendre-transform", "Trans Fast Legendre Transform method"));

        options_.push_back(new SimpleOption<std::string>("plan", "String containing a plan definition"));
        options_.push_back(new SimpleOption<eckit::PathName>("plan-script", "File containing a plan definition"));

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


    if(args.has("plan") || args.has("plan-script")) {
        job.set("style", "custom");
    }


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

    eckit::ScopedPtr<mir::input::MIRInput> input(mir::input::MIRInputFactory::build(args(0)));
    mir::output::GribFileOutput output(args(1));

    // std::string path_lat, path_lon;
    // ASSERT(args.has("latitudes") ==  args.has("longitudes"));
    // if (args.get("latitudes", path_lat) &&  args.get("longitudes", path_lon)) {
    //     input->setAuxilaryFiles(path_lat, path_lon);
    // }

    process(job, *input, output, "field");
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

