/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>
#include <ostream>
#include <string>

#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/filter/NablaFilter.h"
#include "mir/action/plan/Executor.h"
#include "mir/api/MIRJob.h"
#include "mir/caching/matrix/MatrixLoader.h"
#include "mir/data/Space.h"
#include "mir/input/MultiDimensionalGribFileInput.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/intgrid/Intgrid.h"
#include "mir/key/packing/Packing.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/key/truncation/Truncation.h"
#include "mir/lsm/LSMSelection.h"
#include "mir/lsm/NamedLSM.h"
#include "mir/method/Method.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/method/nonlinear/NonLinear.h"
#include "mir/output/MIROutput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/search/Tree.h"
#include "mir/stats/Distribution.h"
#include "mir/stats/Field.h"
#include "mir/stats/Statistics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/SpectralOrder.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"

#if defined mir_HAVE_ATLAS
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/method/fe/FiniteElement.h"
#endif

#if defined(mir_HAVE_PNG)
#include "mir/output/PNGOutput.h"
#endif


using namespace mir;


struct MIR : tools::MIRTool {
    MIR(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        //==============================================
        options_.push_back(new Separator("Spectral transforms"));
        options_.push_back(new FactoryOption<key::truncation::TruncationFactory>(
            "truncation", "Describes the intermediate truncation which the transform is performed from"));
        options_.push_back(new FactoryOption<key::intgrid::IntgridFactory>(
            "intgrid", "Describes the intermediate grid which the transform is performed to"));

        options_.push_back(new SimpleOption<bool>(
            "vod2uv",
            "Input is vorticity and divergence (vo/d), convert to Cartesian components (gridded u/v or spectral U/V)"));
        options_.push_back(new FactoryOption<util::SpectralOrderFactory>(
            "spectral-order", "Spectral/gridded transform order of accuracy)"));
        options_.push_back(new SimpleOption<bool>("atlas-trans-flt", "Atlas/Trans Fast Legendre Transform"));
        options_.push_back(new SimpleOption<std::string>("atlas-trans-type",
                                                         "Atlas/Trans spectral transforms type (default 'local')"));

        //==============================================
        options_.push_back(new Separator("Interpolation"));
        options_.push_back(new FactoryOption<key::grid::GridPattern>(
            "grid", "Interpolate to given grid (following a recognizable regular expression)"));
        options_.push_back(new SimpleOption<size_t>("regular",
                                                    "Interpolate to the regular Gaussian grid N (regular_gg), with N "
                                                    "the number of parallels between pole and equator (N>=2)"));
        options_.push_back(
            new SimpleOption<size_t>("reduced",
                                     "Interpolate to the reduced Gaussian grid N (reduced_gg), with N the number of "
                                     "parallels between pole and equator (pre-defined list of N values.) N must be 16, "
                                     "24, 32, 48, 64, 80, 96, 128, 160, 200, 256, 320, 400, 512, 576, 640 or 800"));
        options_.push_back(
            new SimpleOption<size_t>("octahedral",
                                     "Interpolate to the octahedral reduced Gaussian grid N (reduced_gg), with N the "
                                     "number of parallels between pole and equator (N>=2)"));
        options_.push_back(
            new VectorOption<long>("pl", "Interpolate to the reduced Gaussian grid with specific pl array", 0));
        options_.push_back(
            new VectorOption<double>("rotation", "Rotate the grid by moving the South pole to latitude/longitude", 2));

        options_.push_back(
            new FactoryOption<method::MethodFactory>("interpolation", "Grid to grid interpolation method"));

        options_.push_back(
            new FactoryOption<stats::FieldFactory>("interpolation-statistics", "Statistics interpolation method"));

        options_.push_back(
            new SimpleOption<bool>("interpolation-matrix-free", "Matrix-free interpolation (proxy methods)"));

#if defined(mir_HAVE_ATLAS)
        options_.push_back(new FactoryOption<method::fe::FiniteElementFactory>("l2-projection-input-method",
                                                                               "L2 Projection FE method for input"));
        options_.push_back(new FactoryOption<method::fe::FiniteElementFactory>("l2-projection-output-method",
                                                                               "L2 Projection FE method for output"));
        options_.push_back(new SimpleOption<bool>("finite-element-validate-mesh",
                                                  "FE method check mesh quadrilaterals validity (default false)"));
        options_.push_back(
            new SimpleOption<bool>("finite-element-missing-value-on-projection-fail",
                                   "FE method sets missing value when interpolation isn't possible (default true)"));
#endif

        options_.push_back(new FactoryOption<method::nonlinear::NonLinearFactory>(
            "non-linear",
            "Non-linear treatment on the interpolation linear system (such as the handling of missing values)"));
        options_.push_back(new SimpleOption<double>("simulated-missing-value",
                                                    "specific value to handle as missing (avoid interpolation)"));
        options_.push_back(new SimpleOption<double>("simulated-missing-value-epsilon",
                                                    "specific value to handle as missing, tolerance"));

        options_.push_back(
            new SimpleOption<bool>("uv2uv", "Input is vector (gridded u/v or spectral U/V), convert to gridded u/v"));
        options_.push_back(new SimpleOption<bool>("u-only", "Keep only specific component ('uv2uv'/'vod2uv')"));
        options_.push_back(new SimpleOption<bool>("v-only", "Keep only specific component ('uv2uv'/'vod2uv')"));
        options_.push_back(
            new SimpleOption<size_t>("only", "Filter input on parameter id (incompatible with 'u-only'/'v-only')"));

        options_.push_back(new SimpleOption<eckit::PathName>(
            "same", "Interpolate to the same grid type as the first GRIB message in file"));
        options_.push_back(new SimpleOption<bool>("filter", "Interpolation filter, keeping the same input grid type"));

        options_.push_back(new FactoryOption<method::knn::pick::PickFactory>(
            "nearest-method", "Neighbour picking method, used by k-nearest methods"));
        options_.push_back(new SimpleOption<size_t>(
            "nclosest", "Number of points neighbours to weight (k), used by k-nearest methods"));
        options_.push_back(new SimpleOption<double>(
            "distance", "Radius [m] of neighbours to weight (k), used by k-nearest methods (default 1.)"));
        options_.push_back(new FactoryOption<method::knn::distance::DistanceWeightingFactory>(
            "distance-weighting", "Distance weighting method, used by k-nearest methods"));
        options_.push_back(new FactoryOption<method::knn::distance::DistanceWeightingWithLSMFactory>(
            "distance-weighting-with-lsm", "Distance weighting with land-sea mask, used by nearest-lsm method"));
        options_.push_back(new SimpleOption<double>(
            "distance-tolerance",
            "Distance tolerance when checking distinguishing the nearest neighbours (default 1.)"));
        options_.push_back(
            new SimpleOption<double>("distance-weighting-gaussian-stddev",
                                     "Distance weighting Gaussian function standard deviation [m] (default 1.)"));
        options_.push_back(new SimpleOption<double>("distance-weighting-shepard-power",
                                                    "Distance weighting Shepard power parameter (default 2.)"));
        options_.push_back(new SimpleOption<double>("climate-filter-delta",
                                                    "Climate filter (topographic data smoothing operator) width of "
                                                    "filter edge, must be greater than 'distance' (default 1000.)"));

        options_.push_back(new SimpleOption<bool>("caching", "Caching of weights and grids (default 1)"));
        options_.push_back(new FactoryOption<eckit::linalg::LinearAlgebra>(
            "backend", "Linear algebra backend (default '" + eckit::linalg::LinearAlgebra::backend().name() + "')"));
        options_.push_back(new FactoryOption<search::TreeFactory>("point-search-trees", "k-d tree control"));

#if defined(mir_HAVE_ATLAS)
        for (const std::string& which : {"input", "output"}) {
            options_.push_back(
                new SimpleOption<std::string>(which + "-mesh-generator", "Mesh generator for " + which + " grid"));
            options_.push_back(
                new SimpleOption<bool>(which + "-mesh-cell-centres", "Calculate cell centres for " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-cell-longest-diagonal",
                                                      "Calculate cells longest diagonal for " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-node-lumped-mass-matrix",
                                                      "Calculate node-lumped mass matrix for " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-node-to-cell-connectivity",
                                                      "Calculate node-to-cell connectivity for " + which + " mesh"));
            options_.push_back(new SimpleOption<std::string>(
                which + "-mesh-file-ll",
                "Output file for " + which + " grid, in lon/lat coordinates (default <empty>)"));
            options_.push_back(new SimpleOption<std::string>(
                which + "-mesh-file-xy", "Output file for " + which + " grid, in X/Y coordinates (default <empty>)"));
            options_.push_back(new SimpleOption<std::string>(
                which + "-mesh-file-xyz",
                "Output file for " + which + " grid, in X/Y/Z coordinates (default <empty>)"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-three-dimensional",
                                                      "Generate 3-dimensional " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-triangulate",
                                                      "Generate triangulated " + which + " mesh"));
            options_.push_back(new SimpleOption<double>(
                which + "-mesh-generator-angle", "Generate with quadrilateral tolerance angle on " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-force-include-north-pole",
                                                      "Generate including North pole on " + which + " mesh"));
            options_.push_back(new SimpleOption<bool>(which + "-mesh-generator-force-include-south-pole",
                                                      "Generate including South pole on " + which + " mesh"));
        }
#endif

        options_.push_back(
            new SimpleOption<double>("counter-upper-limit", "Statistics count values below lower limit"));
        options_.push_back(
            new SimpleOption<double>("counter-lower-limit", "Statistics count values above upper limit"));

        options_.push_back(new SimpleOption<bool>(
            "mode-disambiguate-max", "Statistics mode disambiguate with maximum (default, otherwise minimum)"));
        options_.push_back(new VectorOption<double>("mode-boolean-min",
                                                    "Statistics mode boolean threshold min <= value (default 0.5)", 0));
        options_.push_back(
            new VectorOption<double>("mode-real-min", "Statistics mode bin ranges min <= value (default 0.5)", 0));
        options_.push_back(
            new VectorOption<double>("mode-real-values", "Statistics mode bin ranges value (default 0/1)", 0));

        //==============================================
        options_.push_back(new Separator("Filtering"));
        options_.push_back(new VectorOption<double>("area", "cropping area: north/west/south/east", 4));
        options_.push_back(new SimpleOption<eckit::PathName>("bitmap", "Bitmap file to apply"));
        options_.push_back(new SimpleOption<size_t>("frame", "Size of the frame"));
        options_.push_back(new FactoryOption<stats::DistributionFactory>(
            "add-random", "Add random numbers to field values according to a probability density function"));

        options_.push_back(new FactoryOption<action::NablaFilterFactory>("nabla", "Vector/scalar operator(s)"));

        options_.push_back(new SimpleOption<bool>("nabla-poles-missing-values", "Force missing values at the poles"));
        options_.push_back(new SimpleOption<bool>(
            "pre-globalise", "Make the field global (before interpolation) adding missing values if needed"));
        options_.push_back(new SimpleOption<bool>(
            "globalise", "Make the field global (after interpolation) adding missing values if needed"));
        options_.push_back(new SimpleOption<std::string>("globalise-gridname",
                                                         "Unstructured grid globalise using gridname (default O16)"));
        options_.push_back(new SimpleOption<double>(
            "globalise-missing-radius",
            "Unstructured grid globalise minimum distance to insert missing values if needed (default 555975. [m])"));
        options_.push_back(new SimpleOption<bool>("unstructured", "Convert to unstructured grid"));

        options_.push_back(new SimpleOption<bool>("cesaro", "Cesàro summation filtering"));
        options_.push_back(new SimpleOption<double>("cesaro-k", "Cesàro summation k (default 2.)"));
        options_.push_back(new SimpleOption<size_t>(
            "cesaro-truncation", "Cesàro summation filtering minimum truncation (1 <= Tmin < T, default 1)"));

        //==============================================
        options_.push_back(new Separator("Compute"));
        options_.push_back(new SimpleOption<std::string>("formula", "Formula to apply on field"));

        //==============================================
        options_.push_back(new Separator("Land-sea mask handling"));
        options_.push_back(new SimpleOption<bool>("lsm", "Use land-sea mask (lsm) when interpolating grid to grid"));

        options_.push_back(new SimpleOption<double>("lsm-weight-adjustment",
                                                    "LSM interpolation weight adjustment factor (default 0.2)"));

        for (const std::string& io : {"", "input", "output"}) {
            const std::string which = io.empty() ? "both input and output" : io;
            const std::string key   = (io.empty() ? "" : "-") + io;
            options_.push_back(new FactoryOption<method::MethodFactory>(
                "lsm-interpolation" + key, "LSM interpolation method for " + which + ", default nearest-neighbour"));
            options_.push_back(
                new FactoryOption<lsm::LSMSelection>("lsm-selection" + key, "LSM selection method for " + which));
            options_.push_back(new FactoryOption<lsm::NamedMaskFactory>(
                "lsm-named" + key, "If --lsm-selection" + key + "=named, LSM name to use for " + which));
            options_.push_back(new SimpleOption<eckit::PathName>(
                "lsm-file" + key, "If --lsm-selection" + key + "=file, LSM GRIB file path to use for " + which));
            options_.push_back(new SimpleOption<double>(
                "lsm-value-threshold" + key,
                "If --lsm-selection" + key +
                    "=file, LSM field greater-or-equal to value threshold, when converting to mask for " + which +
                    " (default 0.5)"));
        }

        //==============================================
        options_.push_back(new Separator("Unstructured grids"));
        options_.push_back(
            new SimpleOption<eckit::PathName>("griddef", "GRIB file with latitude/longitude pairs (output)"));
        options_.push_back(new SimpleOption<eckit::PathName>("latitudes", "GRIB file with latitudes (output)"));
        options_.push_back(new SimpleOption<eckit::PathName>("longitudes", "GRIB file with longitudes (output)"));

        //==============================================
        options_.push_back(new Separator("GRIB Output"));
        options_.push_back(new SimpleOption<size_t>("accuracy", "Number of bits per value"));
        options_.push_back(new FactoryOption<key::packing::PackingFactory>("packing", "GRIB packing method"));
        options_.push_back(new SimpleOption<size_t>("edition", "GRIB edition number"));

        options_.push_back(new SimpleOption<bool>("delete-local-definition", "Remove GRIB local extension"));
        options_.push_back(
            new SimpleOption<std::string>("metadata", "Set eccodes keys to integer values (a=b,c=d,..)"));

        //==============================================
        options_.push_back(new Separator("Statistics"));
        options_.push_back(new FactoryOption<stats::StatisticsFactory>(
            "statistics", "Statistics methods for interpreting field values (both pre- and post-processed)"));
        options_.push_back(new FactoryOption<stats::StatisticsFactory>(
            "input-statistics", "Statistics methods for interpreting field values (pre-processed)"));
        options_.push_back(new FactoryOption<stats::StatisticsFactory>(
            "output-statistics", "Statistics methods for interpreting field values (post-processed)"));
        options_.push_back(new FactoryOption<stats::DistributionFactory>(
            "distribution", "Generate random numbers according to a probability density function"));

        //==============================================
        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(
            new FactoryOption<key::style::MIRStyleFactory>("style", "Select how the interpolations are performed"));
        options_.push_back(new FactoryOption<data::SpaceChooser>("dimension", "Select dimension"));
        options_.push_back(new SimpleOption<size_t>("precision", "Statistics methods output precision"));
        options_.push_back(new SimpleOption<std::string>("input", "Input options YAML (lat, lon, etc.)"));
        options_.push_back(new SimpleOption<std::string>("output", "Output options YAML"));
        options_.push_back(new FactoryOption<action::Executor>("executor", "Select whether threads are used or not"));
        options_.push_back(new SimpleOption<std::string>("plan", "String containing a plan definition"));
        options_.push_back(new SimpleOption<eckit::PathName>("plan-script", "File containing a plan definition"));

        //==============================================
        options_.push_back(new Separator("Caching"));
        options_.push_back(new FactoryOption<caching::matrix::MatrixLoaderFactory>(
            "matrix-loader", "Select how to load matrices in memory"));
#if defined(mir_HAVE_ATLAS)
        options_.push_back(new FactoryOption<caching::legendre::LegendreLoaderFactory>(
            "legendre-loader", "Select how to load Legendre coefficients in memory"));
#endif

        //==============================================
        // Only show these options if debug channel is active
        if (Log::debug()) {
            options_.push_back(new Separator("Debugging"));
            options_.push_back(new SimpleOption<bool>(
                "dryrun", "Only read data from source, no interpolation done or output produced"));
            options_.push_back(new SimpleOption<bool>("checkerboard", "Create checkerboard field"));
            options_.push_back(new SimpleOption<bool>("pattern", "Create reference pattern field"));
            options_.push_back(new SimpleOption<size_t>("param-id", "Set parameter id"));
            options_.push_back(new SimpleOption<bool>("0-1", "Set pattern and checkerboard values between 0 and 1"));
            options_.push_back(new VectorOption<long>("frequencies", "Set pattern and checkerboard frequencies", 2));
            options_.push_back(
                new SimpleOption<std::string>("dump-plan-file", "Write plan to file (before plan execution)"));
            options_.push_back(new SimpleOption<std::string>("dump-statistics-file",
                                                             "Write statistics to file (after plan execution)"));
            options_.push_back(new SimpleOption<bool>("dont-compress-plan", "Don't compress plan"));
            options_.push_back(new FactoryOption<output::MIROutputFactory>("format", "Output format"));
#if defined(mir_HAVE_PNG)
            options_.push_back(
                new FactoryOption<output::PNGEncoderFactory>("png-output-encoder", "PNG output encoder"));
            options_.push_back(new VectorOption<double>("png-output-minmax", "PNG output minimum/maximum", 2));
#endif
        }
    }

    int minimumPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool
                    << " [--key1=value [--key2=value [...]]] input.grib output.grib"
                       "\n"
                       "Examples: "
                       "\n"
                       "  % "
                    << tool
                    << " --grid=2/2 --area=90/-8/12/80 input.grib output.grib"
                       "\n"
                       "  % "
                    << tool
                    << " --reduced=80 input.grib output.grib"
                       "\n"
                       "  % "
                    << tool
                    << " --regular=80 input.grib output.grib"
                       "\n"
                       "  % "
                    << tool << " --truncation=63 input.grib output.grib" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;

    void process(const api::MIRJob&, input::MIRInput&, output::MIROutput&, const std::string&);

    void only(const api::MIRJob&, input::MIRInput&, output::MIROutput&, const std::string&, size_t);
};


void MIR::execute(const eckit::option::CmdArgs& args) {
    trace::ResourceUsage usage("mir");
    const param::ConfigurationWrapper args_wrap(args);

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
        std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(filter ? args(0) : same, args_wrap));
        ASSERT(input->next());
        job.representationFrom(*input);
    }


    if (args.has("plan") || args.has("plan-script")) {
        job.set("style", "custom");
    }


    std::unique_ptr<output::MIROutput> output(output::MIROutputFactory::build(args(1), args_wrap));
    ASSERT(output);

    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(args(0), args_wrap));
    ASSERT(input);

    auto what = std::to_string(input->dimensions()) + "-dimensional field";

    size_t onlyParamId = 0;
    if (args.get("only", onlyParamId)) {
        only(job, *input, *output, what, onlyParamId);
        return;
    }

    process(job, *input, *output, what);
}


void MIR::process(const api::MIRJob& job, input::MIRInput& input, output::MIROutput& output, const std::string& what) {
    trace::Timer timer("Total time");

    util::MIRStatistics statistics;
    Log::debug() << "Using '" << eckit::linalg::LinearAlgebra::backend().name() << "' backend." << std::endl;

    size_t i = 0;
    while (input.next()) {
        Log::debug() << "============> " << what << ": " << (++i) << std::endl;
        job.execute(input, output, statistics);
    }

    statistics.report(Log::info());

    Log::info() << Log::Pretty(i, what) << " in " << timer.elapsedSeconds() << ", rate: " << double(i) / timer.elapsed()
                << " " << what << "/s" << std::endl;
}


void MIR::only(const api::MIRJob& job, input::MIRInput& input, output::MIROutput& output, const std::string& what,
               size_t paramId) {
    trace::Timer timer("Total time");

    util::MIRStatistics statistics;
    Log::debug() << "Using '" << eckit::linalg::LinearAlgebra::backend().name() << "' backend." << std::endl;

    size_t i = 0;
    while (input.only(paramId)) {
        Log::debug() << "============> paramId: " << paramId << ": " << (++i) << std::endl;
        job.execute(input, output, statistics);
    }

    statistics.report(Log::info());

    Log::info() << Log::Pretty(i, what) << " in " << timer.elapsedSeconds() << ", rate: " << double(i) / timer.elapsed()
                << " " << what << "/s" << std::endl;
}


int main(int argc, char** argv) {
    MIR tool(argc, argv);
    return tool.start();
}
