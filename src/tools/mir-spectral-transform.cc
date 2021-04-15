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


#include <algorithm>
#include <memory>

#include "eckit/filesystem/PathName.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MultiScalarInput.h"
#include "mir/key/grid/Grid.h"
#include "mir/output/MIROutput.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Rotation.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"
#include "mir/util/Wind.h"


using namespace mir;


struct MIRSpectralTransform : tools::MIRTool {
    MIRSpectralTransform(int argc, char** argv) : MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new Separator("Output grid (mandatory one option)"));
        options_.push_back(
            new VectorOption<double>("grid", "Regular latitude/longitude grid increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<std::string>("gridname", "Interpolate to given grid name"));
        options_.push_back(
            new SimpleOption<eckit::PathName>("griddef", "Path to file containing latitude/longitude pairs"));

        options_.push_back(new Separator("Output regular latitude/longitude grids"));
        options_.push_back(new VectorOption<double>(
            "area", "Regular latitude/longitude grid bounding box (North/West/South/East)", 4));
        options_.push_back(new VectorOption<double>(
            "rotation", "Regular latitude/longitude grid rotation by moving the South pole to latitude/longitude", 2));

        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(new SimpleOption<bool>(
            "vod2uv",
            "Input is vorticity and divergence (vo/d), convert to u/v components (gridded u/v or spectral U/V)"));
        options_.push_back(new SimpleOption<size_t>(
            "multi-scalar", "Number of fields (scalar or vo/d pairs) per Atlas/Trans instance (default 1)"));
        options_.push_back(new SimpleOption<size_t>(
            "multi-transform",
            "Number of fields  (scalar or vo/d pairs) per inverse transform (default is value of 'multi-scalar')"));
        options_.push_back(new SimpleOption<std::string>("atlas-trans-type",
                                                         "Atlas/Trans spectral transforms type (default 'local')"));
        options_.push_back(new SimpleOption<bool>("cesaro", "Cesàro summation filtering"));
        options_.push_back(new SimpleOption<double>("cesaro-k", "Cesàro summation k (default 2.)"));
        options_.push_back(new SimpleOption<size_t>(
            "cesaro-truncation", "Cesàro summation filtering minimum truncation (1 <= Tmin < T, default 1)"));
        options_.push_back(new SimpleOption<bool>("unstructured", "Atlas: force unstructured grid (default false)"));
        options_.push_back(new SimpleOption<bool>("caching", "MIR: caching (default true)"));
        options_.push_back(new SimpleOption<bool>("validate", "MIR: validate results (default false)"));
    }

    int minimumPositionalArguments() const override { return 2; }

    void usage(const std::string& tool) const override {
        Log::info() << "\n"
                       "Usage: "
                    << tool
                    << " --grid=WE/SN|--gridname=<namedgrid>|--griddef=<path> [--key1=value [--key2=value "
                       "[...]]] input.grib output.grib"
                       "\n"
                       "Examples: "
                       "\n"
                       "  % "
                    << tool
                    << " --grid=1/1 --area=90/-180/-90/179 in out"
                       "\n"
                       "  % "
                    << tool
                    << " --gridname=O32 --validate=false in out"
                       "\n"
                       "  % "
                    << tool << " --griddef=weather-params.pts in out" << std::endl;
    }

    void execute(const eckit::option::CmdArgs&) override;
};

const repres::Representation* output_representation(std::ostream& log,
                                                    const param::MIRParametrisation& parametrisation) {

    std::vector<double> grid;
    if (parametrisation.get("grid", grid)) {

        ASSERT(grid.size() == 2);
        util::Increments increments(grid[0], grid[1]);

        util::BoundingBox bbox;
        std::vector<double> area;
        if (parametrisation.get("area", area)) {
            ASSERT(area.size() == 4);
            bbox = util::BoundingBox(area[0], area[1], area[2], area[3]);
        }

        log << "MIRSpectralTransform:"
            << "\n\t" << increments << "\n\t" << bbox
            << "\n\t"
               "shifted in latitude? "
            << increments.isLatitudeShifted(bbox)
            << "\n\t"
               "shifted in longitude? "
            << increments.isLongitudeShifted(bbox) << std::endl;

        std::vector<double> rot;
        if (parametrisation.get("rotation", rot)) {
            ASSERT(rot.size() == 2);
            util::Rotation rotation(rot[0], rot[1]);

            return new repres::latlon::RotatedLL(increments, rotation, bbox);
        }

        return new repres::latlon::RegularLL(increments, bbox);
    }

    std::string griddef;
    if (parametrisation.get("griddef", griddef) && !griddef.empty()) {
        ASSERT(!parametrisation.has("rotation"));
        return new repres::other::UnstructuredGrid(eckit::PathName(griddef));
    }

    std::string gridname;
    if (parametrisation.get("gridname", gridname) && !gridname.empty()) {
        ASSERT(!parametrisation.has("rotation"));
        return key::grid::Grid::lookup(gridname).representation();
    }

    throw exception::UserError("MIRSpectralTransform: could not create output representation");
}

atlas::Grid output_grid(const param::MIRParametrisation& parametrisation,
                        const repres::Representation& representation) {

    if (parametrisation.has("griddef") || parametrisation.has("unstructured")) {
        std::unique_ptr<repres::Iterator> it(representation.iterator());

        auto coordinates = new std::vector<atlas::PointXY>;
        coordinates->reserve(representation.count());

        while (it->next()) {
            const Point2& p(*(*it));
            coordinates->push_back(atlas::PointXY(p[1], p[0]));
        }

        return atlas::UnstructuredGrid(coordinates);
    }

    return representation.atlasGrid();
}

void MIRSpectralTransform::execute(const eckit::option::CmdArgs& args) {
    trace::ResourceUsage usage("MIRSpectralTransform");

    // Setup options
    static param::DefaultParametrisation defaults;
    const param::ConfigurationWrapper commandLine(args);

    long paramIdu = 0;
    long paramIdv = 0;
    util::Wind::paramIds(commandLine, paramIdu, paramIdv);

    const bool vod2uv   = args.getBool("vod2uv", false);
    const bool validate = args.getBool("validate", false);
    const bool cesaro   = args.getBool("cesaro", false);

    const size_t multiScalar = args.getUnsigned("multi-scalar", 1);
    if (multiScalar < 1) {
        throw exception::UserError("Option 'multi-scalar' has to be greater than or equal to one");
    }

    size_t multiTransform = args.getUnsigned("multi-transform", multiScalar);
    if (multiTransform < 1 || multiTransform > multiScalar) {
        throw exception::UserError("Option 'multi-transform' has to be in range [1, " + std::to_string(multiScalar) +
                                   "] ('multi-scalar')");
    }

    if ((args.has("grid") ? 1 : 0) + (args.has("gridname") ? 1 : 0) + (args.has("griddef") ? 1 : 0) != 1) {
        throw exception::UserError("Output description is required: either 'grid', 'gridname' or 'griddef'");
    }

    // Setup output (file)
    std::unique_ptr<output::MIROutput> output(output::MIROutputFactory::build(args(1), commandLine));
    ASSERT(output);

    // Setup input (file) and parametrisation
    std::unique_ptr<input::MIRInput> input;
    {
        auto multi = new input::MultiScalarInput();
        for (size_t i = 0; i < multiScalar; ++i) {
            if (vod2uv) {
                // vo/d field pairs
                multi->appendScalarInput(new input::GribFileInput(args(0), i * 2, multiScalar * 2));
                multi->appendScalarInput(new input::GribFileInput(args(0), i * 2 + 1, multiScalar * 2));
            }
            else {
                multi->appendScalarInput(new input::GribFileInput(args(0), i, multiScalar));
            }
        }
        input.reset(multi);
    }
    ASSERT(input);

    const param::CombinedParametrisation combined(commandLine, input->parametrisation(), defaults);
    auto& parametrisation = dynamic_cast<const param::MIRParametrisation&>(combined);

    // Preserve statistics
    util::MIRStatistics statistics;

    {
        auto& log = Log::info();

        trace::Timer total_timer("Total time");

        const std::string what(vod2uv ? "vo/d field pair" : "field");

        size_t i = 0;
        while (input->next()) {
            log << "============> " << what << ": " << (++i * multiScalar) << std::endl;

            // Set representation and grid
            const repres::Representation* outputRepresentation = output_representation(log, parametrisation);
            ASSERT(outputRepresentation);

            atlas::Grid outputGrid = output_grid(parametrisation, *outputRepresentation);
            ASSERT(outputGrid);

            context::Context ctx(*input, statistics);
            data::MIRField& field = ctx.field();

            size_t T = field.representation()->truncation();
            ASSERT(T > 0);

            size_t N = repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
            ASSERT(N > 0);

            // Cesàro summation filtering
            if (cesaro) {
                trace::Timer timer("time on Cesàro summation filtering", log);

                std::vector<double> filter(T + 1);
                {
                    double k    = args.getDouble("cesaro-k", 2.);
                    size_t Tmin = args.getUnsigned("cesaro-truncation", 1);
                    ASSERT(1 <= Tmin && Tmin < T);

                    std::fill_n(filter.begin(), Tmin, 1.);
                    for (size_t n = Tmin; n <= T; ++n) {
                        auto a    = double(T - n + 1);
                        auto f    = filter[n - 1];
                        filter[n] = f * a / (a + k);
                    }
                }

                for (size_t d = 0; d < field.dimensions(); ++d) {
                    auto& values = field.direct(d);
                    ASSERT(values.size() == N * 2);

                    for (size_t m = 0, k = 0; m <= T; ++m) {
                        for (size_t n = m; n <= T; ++n) {
                            ASSERT(k + 1 < N * 2);
                            values[k++] *= filter[n];
                            values[k++] *= filter[n];
                        }
                    }
                }
            }

            // Set Trans
            std::string type = "local";
            parametrisation.get("atlas-trans-type", type);

            atlas::util::Config transConfig;
            transConfig.set("type", type);

            atlas::trans::Trans trans(outputGrid, int(T), transConfig);

            log << "MIRSpectralTransform:"
                << "\n\t"
                   "Atlas/Trans configuration type: "
                << transConfig.getString("type", "(default)")
                << "\n\t"
                   "Atlas unstructured grid: "
                << (atlas::UnstructuredGrid(outputGrid) ? "yes" : "no") << std::endl;

            if (vod2uv) {
                ASSERT(field.dimensions() == multiScalar * 2);

                size_t F;
                for (size_t numberOfFieldPairsProcessed = 0; numberOfFieldPairsProcessed < multiScalar;
                     numberOfFieldPairsProcessed += F) {
                    F = std::min(multiTransform, multiScalar - numberOfFieldPairsProcessed);
                    ASSERT(F > 0);

                    log << "MIRSpectralTransform: transforming " << Log::Pretty(int(F), what) << "..." << std::endl;

                    // set input working area
                    // spectral coefficients are "interlaced", avoid copies if transforming only one field pair)
                    MIRValuesVector input_vo;
                    MIRValuesVector input_d;
                    if (F > 1) {
                        trace::Timer timer("time on interlacing spectra", log);
                        input_vo.resize(F * N * 2);
                        input_d.resize(F * N * 2);

                        for (size_t f = 0; f < F; ++f) {
                            const size_t which = (numberOfFieldPairsProcessed + f) * 2;
                            repres::sh::SphericalHarmonics::interlace_spectra(input_vo, field.values(which + 0), T, N,
                                                                              f, F);
                            repres::sh::SphericalHarmonics::interlace_spectra(input_d, field.values(which + 1), T, N, f,
                                                                              F);
                        }
                    }

                    // set output working area
                    auto Ngp = size_t(outputGrid.size());
                    std::vector<double> out(F * Ngp * 2);

                    // inverse transform
                    {
                        trace::Timer timer("time on invtrans", log);
                        const size_t which = numberOfFieldPairsProcessed * 2;
                        trans.invtrans(int(F), F > 1 ? input_vo.data() : field.values(which + 0).data(),
                                       F > 1 ? input_d.data() : field.values(which + 1).data(), out.data(),
                                       atlas::option::global());
                    }

                    // set field values, forcing u/v paramId (copies are necessary since fields are paired)
                    // Note: transformed u and v fields are contiguous, we save them in alternate order
                    {
                        trace::Timer timer("time on copying grid-point values", log);

                        auto u = paramIdu;
                        auto v = paramIdv;

                        auto here = out.cbegin();
                        for (size_t f = 0; f < F; ++f) {
                            const size_t which = (numberOfFieldPairsProcessed + f) * 2;
                            MIRValuesVector output_field(here, here + int(Ngp));

                            field.update(output_field, which);
                            field.metadata(which, "paramId", u);
                            here += int(Ngp);
                        }

                        for (size_t f = 0; f < F; ++f) {
                            const size_t which = (numberOfFieldPairsProcessed + f) * 2 + 1;
                            MIRValuesVector output_field(here, here + int(Ngp));

                            field.update(output_field, which);
                            field.metadata(which, "paramId", v);
                            here += int(Ngp);
                        }
                    }
                }
            }
            else {
                ASSERT(field.dimensions() == multiScalar);

                size_t F;
                for (size_t numberOfFieldsProcessed = 0; numberOfFieldsProcessed < multiScalar;
                     numberOfFieldsProcessed += F) {
                    F = std::min(multiTransform, multiScalar - numberOfFieldsProcessed);
                    ASSERT(F > 0);

                    log << "MIRSpectralTransform: transforming " << Log::Pretty(int(F), what) << "..." << std::endl;

                    // set input working area
                    // spectral coefficients are "interlaced", avoid copies if transforming only one field)
                    MIRValuesVector in;
                    if (F > 1) {
                        trace::Timer timer("time on interlacing spectra", log);
                        in.resize(F * N * 2);

                        for (size_t f = 0; f < F; ++f) {
                            repres::sh::SphericalHarmonics::interlace_spectra(
                                in, field.values(numberOfFieldsProcessed + f), T, N, f, F);
                        }
                    }

                    // set output working area
                    const auto Ngp = outputGrid.size();
                    MIRValuesVector out(F * size_t(Ngp));

                    // inverse transform
                    {
                        trace::Timer timer("time on invtrans", log);
                        trans.invtrans(int(F), F > 1 ? in.data() : field.values(numberOfFieldsProcessed).data(),
                                       out.data(), atlas::option::global());
                    }

                    // set field values (again, avoid copies for one field only)
                    if (F > 1) {
                        trace::Timer timer("time on copying grid-point values", log);

                        auto here = out.cbegin();
                        for (size_t f = 0; f < F; ++f) {
                            MIRValuesVector output_field(here, here + Ngp);

                            field.update(output_field, numberOfFieldsProcessed + f);
                            here += Ngp;
                        }
                    }
                    else {
                        field.update(out, numberOfFieldsProcessed);
                    }
                }
            }

            // set field representation
            field.representation(outputRepresentation);
            if (validate) {
                trace::Timer timer("time on validate", log);
                field.validate();
            }

            // save
            output->save(parametrisation, ctx);
        }

        statistics.report(log);

        log << Log::Pretty(int(i * multiScalar), what) << " in " << total_timer.elapsedSeconds()
            << ", rate: " << double(i) / total_timer.elapsed() << " " << what << "/s" << std::endl;
    }
}

int main(int argc, char** argv) {
    MIRSpectralTransform tool(argc, argv);
    return tool.start();
}
