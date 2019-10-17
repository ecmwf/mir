/*
 * (C) Copyright 1996- ECMWF.
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


#include <iostream>
#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Option.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/plan/Executor.h"
#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/caching/matrix/MatrixLoader.h"
#include "mir/config/LibMir.h"
#include "mir/data/Space.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"
#include "mir/lsm/LSMSelection.h"
#include "mir/lsm/NamedLSM.h"
#include "mir/method/Method.h"
#include "mir/method/fe/FiniteElement.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/mir_ecbuild_config.h"
#include "mir/output/MIROutput.h"
#include "mir/packing/Packer.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/search/Tree.h"
#include "mir/stats/Statistics.h"
#include "mir/style/Intgrid.h"
#include "mir/style/MIRStyle.h"
#include "mir/style/SpectralOrder.h"
#include "mir/style/Truncation.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Pretty.h"

#if defined(HAVE_PNG)
#include "mir/output/PNGOutput.h"
#endif


using namespace mir;


class MIR : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 2;
    }

    void process(api::MIRJob&, input::MIRInput&, output::MIROutput&, const std::string&);

public:

    MIR(int argc, char **argv) : MIRTool(argc, argv) {
        using eckit::option::FactoryOption;
        using eckit::option::Separator;
        using eckit::option::SimpleOption;
        using eckit::option::VectorOption;

        //==============================================
        options_.push_back(new Separator("Spectral transforms"));
        options_.push_back(new FactoryOption<style::TruncationFactory>("truncation", "Describes the intermediate truncation which the transform is performed from"));
        options_.push_back(new FactoryOption<style::IntgridFactory>("intgrid", "Describes the intermediate grid which the transform is performed to"));

        options_.push_back(new SimpleOption<bool>("vod2uv", "Input is vorticity and divergence (vo/d), convert to Cartesian components (gridded u/v or spectral U/V)"));
        options_.push_back(new FactoryOption<style::SpectralOrderFactory>("spectral-order", "Spectral/gridded transform order of accuracy)"));
        options_.push_back(new SimpleOption<bool>("atlas-trans-flt", "Atlas/Trans Fast Legendre Transform"));
        options_.push_back(new SimpleOption<std::string>("atlas-trans-type", "Atlas/Trans spectral transforms type (default 'local')"));

        //==============================================
        options_.push_back(new Separator("Interpolation"));
        options_.push_back(new VectorOption<double>("grid", "Interpolate to a regular latitude/longitude grid (regular_ll), provided the West-East & South-North increments", 2));
        options_.push_back(new SimpleOption<size_t>("regular", "Interpolate to the regular Gaussian grid N (regular_gg), with N the number of parallels between pole and equator (N>=2)"));
        options_.push_back(new SimpleOption<size_t>("reduced", "Interpolate to the reduced Gaussian grid N (reduced_gg), with N the number of parallels between pole and equator (pre-defined list of N values.) N must be 16, 24, 32, 48, 64, 80, 96, 128, 160, 200, 256, 320, 400, 512, 576, 640 or 800"));
        options_.push_back(new SimpleOption<size_t>("octahedral", "Interpolate to the octahedral reduced Gaussian grid N (reduced_gg), with N the number of parallels between pole and equator (N>=2)"));
        options_.push_back(new VectorOption<long>("pl", "Interpolate to the reduced Gaussian grid with specific pl array", 0));
        options_.push_back(new SimpleOption<std::string>("gridname", "Interpolate to given grid name"));
        options_.push_back(new VectorOption<double>("rotation", "Rotate the grid by moving the South pole to latitude/longitude", 2));

        options_.push_back(new FactoryOption<method::MethodFactory>("interpolation", "Grid to grid interpolation method"));

        options_.push_back(new FactoryOption<method::fe::FiniteElementFactory>("conservative-finite-element-method-input", "Conservative FEM for input mesh"));
        options_.push_back(new FactoryOption<method::fe::FiniteElementFactory>("conservative-finite-element-method-output", "Conservative FEM for output mesh"));

        options_.push_back(new FactoryOption<method::nonlinear::NonLinearFactory>("non-linear", "Non-linear treatment on the interpolation linear system (such as the handling of missing values)"));
        options_.push_back(new SimpleOption<double>("simulated-missing-value", "specific value to handle as missing (avoid interpolation)"));
        options_.push_back(new SimpleOption<double>("simulated-missing-value-epsilon", "specific value to handle as missing, tolerance"));

        options_.push_back(new SimpleOption<bool>("uv2uv", "Input is vector (gridded u/v or spectral U/V), convert to gridded u/v"));
        options_.push_back(new SimpleOption<bool>("u-only", "Keep only specific component ('uv2uv'/'vod2uv')"));
        options_.push_back(new SimpleOption<bool>("v-only", "Keep only specific component ('uv2uv'/'vod2uv')"));

        options_.push_back(new SimpleOption<eckit::PathName>("same", "Interpolate to the same grid type as the first GRIB message in file"));
        options_.push_back(new SimpleOption<bool>("filter", "Interpolation filter, keeping the same input grid type"));
        options_.push_back(new SimpleOption<eckit::PathName>("griddef", "Path to GRIB file containing a list of latitude/longitude pairs"));

        options_.push_back(new FactoryOption<method::knn::pick::PickFactory>("nearest-method", "Neighbour picking method, used by k-nearest methods"));
        options_.push_back(new SimpleOption<size_t>("nclosest", "Number of points neighbours to weight (k), used by k-nearest methods"));
        options_.push_back(new SimpleOption<double>("distance", "Radius [m] of neighbours to weight (k), used by k-nearest methods (default 1.)"));
        options_.push_back(new FactoryOption<method::knn::distance::DistanceWeightingFactory>("distance-weighting", "Distance weighting method, used by k-nearest methods"));
        options_.push_back(new FactoryOption<method::knn::distance::DistanceWeightingWithLSMFactory>("distance-weighting-with-lsm", "Distance weighting with land-sea mask, used by nearest-lsm method"));
        options_.push_back(new SimpleOption<double>("distance-tolerance", "Distance tolerance when checking distinguishing the nearest neighbours (default 1.)"));
        options_.push_back(new SimpleOption<double>("distance-weighting-gaussian-stddev", "Distance weighting Gaussian function standard deviation [m] (default 1.)"));
        options_.push_back(new SimpleOption<double>("distance-weighting-shepard-power", "Distance weighting Shepard power parameter (default 2.)"));
        options_.push_back(new SimpleOption<double>("climate-filter-delta", "Climate filter (topographic data smoothing operator) width of filter edge, must be greater than 'distance' (default 1000.)"));

        options_.push_back(new SimpleOption<bool>("caching", "Caching of weights and grids (default 1)"));
        options_.push_back(new FactoryOption<eckit::linalg::LinearAlgebra>("backend", "Linear algebra backend (default '" + eckit::linalg::LinearAlgebra::backend().name() + "')"));
        options_.push_back(new FactoryOption<search::TreeFactory>("point-search-trees", "k-d tree control"));

        for (const std::string& which : {"input", "output"}) {
            options_.push_back(new SimpleOption<std::string>(which + "-mesh-generator", "Mesh generator for " + which + " grid"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-cell-centres", "Calculate cell centres for " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-cell-longest-diagonal", "Calculate cells longest diagonal for " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-node-lumped-mass-matrix", "Calculate node-lumped mass matrix for " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-node-to-cell-connectivity", "Calculate node-to-cell connectivity for " + which + " mesh"));
            options_.push_back(new SimpleOption<std::string>(which + "-mesh-file-ll", "Output file for " + which + " grid, in lon/lat coordinates (default <empty>)"));
            options_.push_back(new SimpleOption<std::string>(which + "-mesh-file-xy", "Output file for " + which + " grid, in X/Y coordinates (default <empty>)"));
            options_.push_back(new SimpleOption<std::string>(which + "-mesh-file-xyz", "Output file for " + which + " grid, in X/Y/Z coordinates (default <empty>)"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-three-dimensional", "Generate 3-dimensional " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-triangulate", "Generate triangulated " + which + " mesh"));
            options_.push_back(new SimpleOption<double>(which + "-mesh-generator-angle", "Generate with quadrilateral tolerance angle on " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-force-include-north-pole", "Generate including North pole on " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-force-include-south-pole", "Generate including South pole on " + which + " mesh"));
        }

        //==============================================
        options_.push_back(new Separator("Filtering"));
        options_.push_back(new VectorOption<double>("area", "cropping area: north/west/south/east", 4));
        options_.push_back(new SimpleOption<double>("area-precision", "cropping area precision ('outward')"));
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

        options_.push_back(new SimpleOption<double>("lsm-weight-adjustment", "LSM interpolation weight adjustment factor (default 0.2)"));

        for (const std::string& io : {"", "input", "output"}) {
            const std::string which = io.length() ? io : "both input and output";
            const std::string key = (io.length() ? "-" : "") + io;
            options_.push_back(new FactoryOption<method::MethodFactory>("lsm-interpolation" + key, "LSM interpolation method for " + which + ", default nearest-neighbour"));
            options_.push_back(new FactoryOption<lsm::LSMSelection>("lsm-selection" + key, "LSM selection method for " + which));
            options_.push_back(new FactoryOption<lsm::NamedMaskFactory>("lsm-named" + key, "If --lsm-selection" + key + "=named, LSM name to use for " + which));
            options_.push_back(new SimpleOption<eckit::PathName>("lsm-file" + key, "If --lsm-selection" + key + "=file, LSM grib file path to use for " + which));
            options_.push_back(new SimpleOption<double>("lsm-value-threshold" + key, "If --lsm-selection" + key + "=file, LSM field greater-or-equal to value threshold, when converting to mask for " + which + " (default 0.5)"));
        }

        //==============================================
        options_.push_back(new Separator("Unstructured grids"));
        options_.push_back(new SimpleOption<eckit::PathName>("latitudes", "Path GRIB file of latitudes"));
        options_.push_back(new SimpleOption<eckit::PathName>("longitudes", "Path GRIB file of longitudes"));

        //==============================================
        options_.push_back(new Separator("GRIB Output"));
        options_.push_back(new SimpleOption<size_t>("accuracy", "Number of bits per value"));
        options_.push_back(new FactoryOption<packing::Packer>("packing", "GRIB packing method"));
        options_.push_back(new SimpleOption<size_t>("edition", "GRIB edition number"));

        options_.push_back(new SimpleOption<bool>("delete-local-definition", "Remove GRIB local extension"));
        options_.push_back(new SimpleOption<std::string>("metadata", "Set eccodes keys to integer values (a=b,c=d,..)"));

        //==============================================
        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(new FactoryOption<style::MIRStyleFactory>("style", "Select how the interpolations are performed"));
        options_.push_back(new FactoryOption<data::SpaceChooser>("dimension", "Select dimension"));
        options_.push_back(new FactoryOption<stats::StatisticsFactory>("statistics", "Statistics methods for interpreting field values (both pre- and post-processed)"));
        options_.push_back(new FactoryOption<stats::StatisticsFactory>("input-statistics", "Statistics methods for interpreting field values (pre-processed)"));
        options_.push_back(new FactoryOption<stats::StatisticsFactory>("output-statistics", "Statistics methods for interpreting field values (post-processed)"));
        options_.push_back(new SimpleOption<size_t>("precision", "Statistics methods output precision"));

        options_.push_back(new FactoryOption<action::Executor>("executor", "Select whether threads are used or not"));
        options_.push_back(new SimpleOption<std::string>("plan", "String containing a plan definition"));
        options_.push_back(new SimpleOption<eckit::PathName>("plan-script", "File containing a plan definition"));

        //==============================================
        options_.push_back(new Separator("Caching"));
        options_.push_back(new FactoryOption<caching::matrix::MatrixLoaderFactory>("matrix-loader", "Select how to load matrices in memory"));
        options_.push_back(new FactoryOption<caching::legendre::LegendreLoaderFactory>("legendre-loader", "Select how to load legendre coefficients in memory"));

        //==============================================
        // Only show these options if debug channel is active
        if (eckit::Log::debug<LibMir>()) {
            options_.push_back(new Separator("Debugging"));
            options_.push_back(new SimpleOption<bool>("dummy", "Use dummy data"));
            options_.push_back(new SimpleOption<bool>("dryrun", "Only read data from source, no interpolation done or output produced"));
            options_.push_back(new SimpleOption<bool>("checkerboard", "Create checkerboard field"));
            options_.push_back(new SimpleOption<bool>("pattern", "Create reference pattern field"));
            options_.push_back(new SimpleOption<size_t>("param-id", "Set parameter id"));
            options_.push_back(new SimpleOption<bool>("0-1", "Set pattern and checkerboard values between 0 and 1"));
            options_.push_back(new VectorOption<long>("frequencies", "Set pattern and checkerboard frequencies", 2));
            options_.push_back(new SimpleOption<std::string>("dump-plan-file", "Dump plan to file"));
            options_.push_back(new SimpleOption<bool>("dont-compress-plan", "Don't compress plan"));
            options_.push_back(new FactoryOption<output::MIROutputFactory>("format", "Output format"));
#if defined(HAVE_PNG)
            options_.push_back(new FactoryOption<output::PNGEncoderFactory>("png-output-encoder", "PNG output encoder"));
            options_.push_back(new VectorOption<double>("png-output-minmax", "PNG output minimum/maximum", 2));
#endif
        }
    }

};


void MIR::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " [--key1=value [--key2=value [...]]] input.grib output.grib"
            "\n" "Examples: "
            "\n" "  % " << tool << " --grid=2/2 --area=90/-8/12/80 input.grib output.grib"
            "\n" "  % " << tool << " --reduced=80 input.grib output.grib"
            "\n" "  % " << tool << " --regular=80 input.grib output.grib"
            "\n" "  % " << tool << " --truncation=63 input.grib output.grib"
            << std::endl;
}


void MIR::execute(const eckit::option::CmdArgs& args) {
    eckit::ResourceUsage usage("mir");

    // If we want to control the backend in MARS/PRODGEN, we can move that to MIRJob
    std::string backend;
    if (args.get("backend", backend)) {
        eckit::linalg::LinearAlgebra::backend(backend);
    }

    api::MIRJob job;
    args.configure(job);

    bool filter = false;
    args.get("filter", filter);

    std::string same;
    if (args.get("same", same) || filter) {
        ASSERT(filter != !same.empty());
        input::GribFileInput input(filter ? args(0) : same);
        ASSERT(input.next());
        job.representationFrom(input);
    }


    bool uv2uv = false;
    bool vod2uv = false;
    args.get("uv2uv", uv2uv);
    args.get("vod2uv", vod2uv);


    if (args.has("plan") || args.has("plan-script")) {
        job.set("style", "custom");
    }


    const param::ConfigurationWrapper args_wrap(args);
    std::unique_ptr<output::MIROutput> output(output::MIROutputFactory::build(args(1), args_wrap));
    ASSERT(output);

    if (vod2uv || uv2uv) {
        ASSERT(vod2uv != uv2uv);
        ASSERT(!args.has("latitudes") && !args.has("longitudes"));

        input::GribFileInput input1(args(0), 0, 2);
        input::GribFileInput input2(args(0), 1, 2);
        input::VectorInput input(input1, input2);

        process(job, input, *output, "wind");
        return;
    }


    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(0), args_wrap));
    ASSERT(input);

    if (args.has("latitudes") || args.has("longitudes")) {
        std::string latitudes, longitudes;
        ASSERT(args.get("latitudes", latitudes));
        ASSERT(args.get("longitudes", longitudes));

        input->setAuxilaryFiles(latitudes, longitudes);
    }

    process(job, *input, *output, "field");
}


void MIR::process(api::MIRJob& job, input::MIRInput& input, output::MIROutput& output, const std::string& what) {
    eckit::Timer timer("Total time");

    util::MIRStatistics statistics;
    eckit::Log::debug<LibMir>() << "Using '" << eckit::linalg::LinearAlgebra::backend().name() << "' backend." << std::endl;

    size_t i = 0;
    while (input.next()) {
        eckit::Log::debug<LibMir>() << "============> " << what << ": " << (++i) << std::endl;
        job.execute(input, output, statistics);
    }

    statistics.report(eckit::Log::info());

    eckit::Log::info() << Pretty(i, what) << " in " << eckit::Seconds(timer.elapsed()) <<
                       ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;
}


int main(int argc, char **argv) {
    MIR tool(argc, argv);
    return tool.start();
}

