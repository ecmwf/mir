/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Andreas Mueller
/// @author Pedro Maciel


#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Plural.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/mpi/Comm.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Option.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"
#include "eckit/testing/Test.h"

#include "atlas/grid.h"
#include "atlas/option.h"
#include "atlas/trans/Trans.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MultiScalarInput.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/output/MIROutput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Rotation.h"


class MIRSpectralTransform : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);

    int minimumPositionalArguments() const { return 2; }

    void usage(const std::string& tool) const {
        eckit::Log::info() <<
                "\n" "Usage: " << tool << " --grid=WE/SN|--gridname=<namedgrid>|--griddef=<path> [--key1=value [--key2=value [...]]] input.grib output.grib"
                "\n" "Examples: "
                "\n" "  % " << tool << " --grid=1/1 --area=90/-180/-90/179 in out"
                "\n" "  % " << tool << " --gridname=O32 --validate=false in out"
                "\n" "  % " << tool << " --griddef=weather-params.pts in out"
                << std::endl;
    }

public:
    MIRSpectralTransform(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new Separator("Output grid (mandatory one option)"));
        options_.push_back(new VectorOption<double>("grid", "Regular latitude/longitude grid increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<std::string>("gridname", "Interpolate to given grid name"));
        options_.push_back(new SimpleOption<eckit::PathName>("griddef", "Path to file containing latitude/longitude pairs"));

        options_.push_back(new Separator("Output regular latitude/longitude grids"));
        options_.push_back(new VectorOption<double>("area", "Regular latitude/longitude grid bounding box (North/West/South/East)", 4));
        options_.push_back(new VectorOption<double>("rotation", "Regular latitude/longitude grid rotation by moving the South pole to latitude/longitude", 2));

        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(new SimpleOption<bool>("vod2uv", "Input is vorticity and divergence (vo/d), convert to u/v components (gridded u/v or spectral U/V)"));
        options_.push_back(new SimpleOption<size_t>("multi-scalar", "Number of fields (scalar or vo/d pairs) per Atlas/Trans instance (default 1)"));
        options_.push_back(new SimpleOption<size_t>("multi-transform", "Number of fields  (scalar or vo/d pairs) per inverse transform (default is value of 'multi-scalar')"));
        options_.push_back(new SimpleOption<bool>("local", "Atlas/Trans: force local transform (default false)"));
        options_.push_back(new SimpleOption<bool>("unstructured", "Atlas: force unstructured grid (default false)"));
        options_.push_back(new SimpleOption<bool>("caching", "MIR: caching (default true)"));
        options_.push_back(new SimpleOption<bool>("validate", "MIR: validate results (default false)"));
    }
};


const mir::repres::Representation* output_representation(const mir::param::MIRParametrisation& parametrisation,
                                                         bool local) {

    std::vector<double> grid;
    if (parametrisation.get("grid", grid)) {
        ASSERT(grid.size() == 2);

        eckit::ScopedPtr<mir::util::Increments> increments;
        increments.reset(new mir::util::Increments(grid[0], grid[1]));
        ASSERT(increments);

        mir::util::BoundingBox boundingBox;
        std::vector<double> area;
        if (parametrisation.get("area", area)) {
            ASSERT(area.size() == 4);
            boundingBox = mir::util::BoundingBox(area[0], area[1], area[2], area[3]);
        } else if (local) {
            eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: bounding box global, lat/lon shift allowed" << std::endl;
            increments->globaliseBoundingBox(boundingBox, true, true);

        } else {
            eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: bounding box global, lat/lon shift not allowed" << std::endl;
            increments->globaliseBoundingBox(boundingBox, false, false);
        }

        bool isLatitudeShifted = !(boundingBox.south().fraction() / increments->south_north()).integer();
        bool isLongitudeShifted = !(boundingBox.west().fraction() / increments->west_east()).integer();
        eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: bounding box shifted: "
                                         << "in latitude? " << (isLatitudeShifted ? "yes" : "no") << ", "
                                         << "in longitude? " << (isLongitudeShifted ? "yes" : "no")
                                         << std::endl;

        std::vector<double> rot;
        if (parametrisation.get("rotation", rot)) {
            ASSERT(rot.size() == 2);
            mir::util::Rotation rotation(rot[0], rot[1]);

            return new mir::repres::latlon::RotatedLL(boundingBox, *increments, rotation);
        }

        return new mir::repres::latlon::RegularLL(boundingBox, *increments);
    }

    std::string griddef;
    if (parametrisation.get("griddef", griddef) && !griddef.empty()) {
        ASSERT(!parametrisation.has("rotation"));
        return new mir::repres::other::UnstructuredGrid(eckit::PathName(griddef));
    }

    std::string gridname;
    if (parametrisation.get("gridname", gridname) && !gridname.empty()) {
        ASSERT(!parametrisation.has("rotation"));
        return mir::namedgrids::NamedGrid::lookup(gridname).representation();
    }

    throw eckit::UserError("MIRSpectralTransform: could not create output representation");
}


atlas::Grid output_grid(const mir::param::MIRParametrisation& parametrisation,
                        const mir::repres::Representation& representation) {
    atlas::Grid outputGrid;

    if (parametrisation.has("griddef") || parametrisation.has("unstructured")) {
        eckit::ScopedPtr<mir::repres::Iterator> it(representation.iterator());

        std::vector<atlas::PointXY>* coordinates = new std::vector<atlas::PointXY>;
        coordinates->reserve(representation.count());

        while (it->next()) {
            const mir::repres::Iterator::point_2d_t& p(*(*it));
            coordinates->push_back(atlas::PointXY(p[1], p[0]));
        }

        return atlas::grid::UnstructuredGrid(coordinates);
    }

    return representation.atlasGrid();
}


void interlace_spectra(
        std::vector<double>& interlacedSpectra,
        const std::vector<double>& spectra,
        size_t truncation,
        size_t numberOfComplexCoefficients,
        size_t index,
        size_t indexTotal ) {
    ASSERT(0 <= index && index < indexTotal);
    ASSERT(numberOfComplexCoefficients * 2 * indexTotal == interlacedSpectra.size());

    if (spectra.size() != numberOfComplexCoefficients * 2) {
        const std::string msg = "MIRSpectralTransform: expected field values size " +
                std::to_string(numberOfComplexCoefficients * 2) +
                " (T=" + std::to_string(truncation) + "), " +
                " got " + std::to_string(spectra.size());
        eckit::Log::error() << msg << std::endl;
        throw eckit::UserError(msg);
    }

    for (size_t j = 0; j < numberOfComplexCoefficients * 2; ++j) {
        interlacedSpectra[ j * indexTotal + index ] = spectra[j];
    }
}


void MIRSpectralTransform::execute(const eckit::option::CmdArgs& args) {
    eckit::ResourceUsage usage("MIRSpectralTransform");

    // Setup options
    const mir::param::ConfigurationWrapper parametrisation(args);

    const long paramIdu = mir::LibMir::instance().configuration().getLong("parameter-id-u", 131);
    const long paramIdv = mir::LibMir::instance().configuration().getLong("parameter-id-v", 132);
    ASSERT(paramIdu > 0);
    ASSERT(paramIdv > 0);

    const bool vod2uv   = args.getBool("vod2uv", false);
    const bool validate = args.getBool("validate", false);
    const bool local    = args.getBool("local", false)
            || args.has("griddef")
            || args.has("unstructured")
            || (args.has("grid") && (args.has("area") || args.has("rotation")));

    const size_t multiScalar = args.getUnsigned("multi-scalar", 1);
    if (multiScalar < 1) {
        throw eckit::UserError("Option 'multi-scalar' has to be greater than one");
    }

    size_t multiTransform = args.getUnsigned("multi-transform", multiScalar);
    if (multiTransform < 1 || multiTransform > multiScalar) {
        throw eckit::UserError("Option 'multi-transform' has to be greater than one, and up to 'multi-scalar' (" + std::to_string(multiScalar) + ")");
    }

    if (args.has("grid") + args.has("gridname") + args.has("griddef") != 1) {
        throw eckit::UserError("Output description is required: either 'grid', 'gridname' or 'griddef'");
    }


    // Setup output (file)
    eckit::ScopedPtr<mir::output::MIROutput> output(mir::output::MIROutputFactory::build(args(1), parametrisation));
    ASSERT(output);


    // Setup input (file)
    eckit::ScopedPtr<mir::input::MIRInput> input;
    {
        auto multi = new mir::input::MultiScalarInput();
        for (size_t i = 0; i < multiScalar; ++i) {
            if (vod2uv) {
                // vo/d field pairs
                multi->appendScalarInput(new mir::input::GribFileInput(args(0), i * 2,     multiScalar * 2));
                multi->appendScalarInput(new mir::input::GribFileInput(args(0), i * 2 + 1, multiScalar * 2));
            } else {
                multi->appendScalarInput(new mir::input::GribFileInput(args(0), i, multiScalar));
            }
        }
        input.reset(multi);
    }
    ASSERT(input);

    // Preserve statistics
    mir::util::MIRStatistics statistics;

    {
        eckit::Timer timer("Total time");

        const std::string what(vod2uv ? "vo/d field pair" : "field");

        size_t i = 0;
        while (input->next()) {
            eckit::Log::info() << "============> " << what << ": " << (++i * multiScalar) << std::endl;

            // Set representation and grid
            const mir::repres::Representation* outputRepresentation = output_representation(parametrisation, local);
            ASSERT(outputRepresentation);

            atlas::Grid outputGrid = output_grid(parametrisation, *outputRepresentation);
            ASSERT(outputGrid);

            mir::context::Context ctx(*input, statistics);
            mir::data::MIRField& field = ctx.field();

            size_t T = field.representation()->truncation();
            ASSERT(T > 0);

            size_t N = mir::repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
            ASSERT(N > 0);

            // Set Trans
            atlas::util::Config transConfig;
            if (local) {
                transConfig.set("type", "local");
            }

            atlas::trans::Trans trans(outputGrid, int(T), transConfig);

            eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: Atlas/Trans configuration type: "
                                             << transConfig.getString("type", "(default)") << std::endl;
            eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: Atlas unstructured grid: "
                                             << (atlas::grid::UnstructuredGrid(outputGrid) ? "yes" : "no") << std::endl;

            if (vod2uv) {
                ASSERT(field.dimensions() == multiScalar * 2);

                size_t F = multiTransform;
                for (size_t numberOfFieldPairsProcessed = 0; numberOfFieldPairsProcessed < multiScalar; numberOfFieldPairsProcessed += F) {
                    F = std::min(multiTransform, multiScalar - numberOfFieldPairsProcessed);
                    ASSERT(F > 0);

                    eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: transforming " << eckit::Plural(int(F), what) << "..." << std::endl;

                    // set input working area
                    // spectral coefficients are "interlaced", avoid copies if transforming only one field pair)
                    std::vector<double> input_vo;
                    std::vector<double> input_d;
                    if (F > 1) {
                        eckit::Timer timer("time on interlacing spectra", eckit::Log::debug<mir::LibMir>());
                        input_vo.resize(F * N * 2);
                        input_d.resize(F * N * 2);

                        for (size_t i = 0; i < F; ++i) {
                            const size_t which = (numberOfFieldPairsProcessed + i) * 2;
                            interlace_spectra(input_vo, field.values(which + 0), T, N, i, F);
                            interlace_spectra(input_d,  field.values(which + 1), T, N, i, F);
                        }
                    }

                    // set output working area
                    const size_t Ngp = outputGrid.size();
                    std::vector<double> output(F * Ngp * 2);

                    // inverse transform
                    {
                        eckit::Timer timer("time on invtrans", eckit::Log::debug<mir::LibMir>());
                        const size_t which = numberOfFieldPairsProcessed * 2;
                        trans.invtrans(
                                    int(F),
                                    F > 1 ? input_vo.data() : field.values(which + 0).data(),
                                    F > 1 ? input_d.data()  : field.values(which + 1).data(),
                                    output.data(),
                                    atlas::option::global() );
                    }

                    // set field values, forcing u/v paramId (copies are necessary since fields are paired)
                    // Note: transformed u and v fields are contiguous, we save them in alternate order
                    {
                        eckit::Timer timer("time on copying grid-point values", eckit::Log::debug<mir::LibMir>());

                        auto here = output.cbegin();
                        for (size_t i = 0; i < F; ++i) {
                            const size_t which = (numberOfFieldPairsProcessed + i) * 2;
                            std::vector<double> output_field(here, here + int(Ngp));

                            field.update(output_field, which);
                            field.metadata(which, "paramId", paramIdu);
                            here += int(Ngp);
                        }

                        for (size_t i = 0; i < F; ++i) {
                            const size_t which = (numberOfFieldPairsProcessed + i) * 2 + 1;
                            std::vector<double> output_field(here, here + int(Ngp));

                            field.update(output_field, which);
                            field.metadata(which, "paramId", paramIdv);
                            here += int(Ngp);
                        }
                    }
                }

            } else {
                ASSERT(field.dimensions() == multiScalar);

                size_t F = multiTransform;
                for (size_t numberOfFieldsProcessed = 0; numberOfFieldsProcessed < multiScalar; numberOfFieldsProcessed += F) {
                    F = std::min(multiTransform, multiScalar - numberOfFieldsProcessed);
                    ASSERT(F > 0);

                    eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: transforming " << eckit::Plural(int(F), what) << "..." << std::endl;

                    // set input working area
                    // spectral coefficients are "interlaced", avoid copies if transforming only one field)
                    std::vector<double> input;
                    if (F > 1) {
                        eckit::Timer timer("time on interlacing spectra", eckit::Log::debug<mir::LibMir>());
                        input.resize(F * N * 2);

                        for (size_t i = 0; i < F; ++i) {
                            interlace_spectra(input, field.values(numberOfFieldsProcessed + i), T, N, i, F);
                        }
                    }

                    // set output working area
                    const size_t Ngp = outputGrid.size();
                    std::vector<double> output(F * Ngp);

                    // inverse transform
                    {
                        eckit::Timer timer("time on invtrans", eckit::Log::debug<mir::LibMir>());
                        trans.invtrans(
                                    int(F),
                                    F > 1 ? input.data() : field.values(numberOfFieldsProcessed).data(),
                                    output.data(),
                                    atlas::option::global() );
                    }

                    // set field values (again, avoid copies for one field only)
                    if (F > 1) {
                        eckit::Timer timer("time on copying grid-point values", eckit::Log::debug<mir::LibMir>());

                        auto here = output.cbegin();
                        for (size_t i = 0; i < F; ++i) {
                            std::vector<double> output_field(here, here + int(Ngp));

                            field.update(output_field, numberOfFieldsProcessed + i);
                            here += int(Ngp);
                        }
                    } else {
                        field.update(output, numberOfFieldsProcessed);
                    }
                }

            }

            // set field representation
            field.representation(outputRepresentation);
            if (validate) {
                eckit::Timer timer("time on validate", eckit::Log::debug<mir::LibMir>());
                field.validate();
            }

            // save
            output->save(parametrisation, ctx);
        }

        statistics.report(eckit::Log::info());

        eckit::Log::info() << eckit::Plural(int(i * multiScalar), what) << " in " << eckit::Seconds(timer.elapsed())
                           << ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;
    }
}


int main(int argc, char** argv) {
    MIRSpectralTransform tool(argc, argv);
    return tool.start();
}
