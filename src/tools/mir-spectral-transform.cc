/*
 * (C) Copyright 1996-2016 ECMWF.
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
#include "atlas/trans/Trans.h"

#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"
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
        options_.push_back(new VectorOption<double> ("grid", "Regular latitude/longitude grid increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<std::string> ("gridname", "Interpolate to given grid name"));
        options_.push_back(new SimpleOption<eckit::PathName> ("griddef", "Path to file containing latitude/longitude pairs"));

        options_.push_back(new Separator("Output regular latitude/longitude grids"));
        options_.push_back(new VectorOption<double> ("area", "Regular latitude/longitude grid bounding box (North/West/South/East)", 4));
        options_.push_back(new VectorOption<double> ("rotation", "Regular latitude/longitude grid rotation by moving the South pole to latitude/longitude", 2));

        options_.push_back(new Separator("Miscellaneous"));
        options_.push_back(new SimpleOption<bool> ("vod2uv", "Input is vorticity and divergence (vo/d), convert to u/v components (gridded u/v or spectral U/V)"));
        options_.push_back(new SimpleOption<bool> ("local", "Atlas/Trans: force local transform (default false)"));
        options_.push_back(new SimpleOption<bool> ("unstructured", "Atlas: force unstructured grid (default false)"));
        options_.push_back(new SimpleOption<bool> ("caching", "MIR: caching (default true)"));
        options_.push_back(new SimpleOption<bool> ("validate", "MIR: validate results (default true)"));
    }
};


const mir::repres::Representation* getOutputRepresentation(const mir::param::MIRParametrisation& parametrisation, bool local) {

    std::vector<double> grid;
    if (parametrisation.get("grid", grid)) {
        ASSERT(grid.size() == 2);

        eckit::ScopedPtr< mir::util::Increments > increments;
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


atlas::Grid getOutputGrid(const mir::param::MIRParametrisation& parametrisation, const mir::repres::Representation& representation) {
    atlas::Grid outputGrid;

    if (parametrisation.has("griddef") || parametrisation.has("unstructured")) {
        eckit::ScopedPtr< mir::repres::Iterator > it(representation.iterator());

        std::vector< atlas::PointXY >* coordinates = new std::vector< atlas::PointXY >;
        coordinates->reserve(representation.count());

        while (it->next()) {
            const mir::repres::Iterator::point_2d_t& p(*(*it));
            coordinates->push_back(atlas::PointXY(p[1], p[0]));
        }

        return atlas::grid::UnstructuredGrid(coordinates);
    }

    return representation.atlasGrid();
}



void MIRSpectralTransform::execute(const eckit::option::CmdArgs& args) {
    eckit::ResourceUsage usage("MIRSpectralTransform");

    // Setup options
    const mir::param::ConfigurationWrapper parametrisation(args);
    if (args.has("grid") + args.has("gridname") + args.has("griddef") != 1) {
        throw eckit::UserError("Output description is required: either 'grid', 'gridname' or 'griddef'");
    }

    bool vod2uv = false;
    args.get("vod2uv", vod2uv);

    bool validate = true;
    args.get("validate", validate);

    bool local = false;
    args.get("local", local);

    local = local
            || args.has("griddef")
            || args.has("unstructured")
            || (args.has("grid") && (args.has("area") || args.has("rotation")));


    // Setup output file
    eckit::ScopedPtr<mir::output::MIROutput> output(mir::output::MIROutputFactory::build(args(1), parametrisation));
    ASSERT(output);


    // Setup input file
    eckit::ScopedPtr< mir::input::GribFileInput > msg1;
    eckit::ScopedPtr< mir::input::GribFileInput > msg2;
    eckit::ScopedPtr< mir::input::MIRInput > input;

    if (vod2uv) {
        msg1.reset(new mir::input::GribFileInput(args(0), 0, 2)); // vo
        msg2.reset(new mir::input::GribFileInput(args(0), 1, 2)); // d
        input.reset(new mir::input::VectorInput(*msg1, *msg2));
    } else {
        input.reset(new mir::input::GribFileInput(args(0)));
    }
    ASSERT(input);


    {
        eckit::Timer timer("Total time");

        const std::string what(vod2uv ? "vo/d field pair" : "field");

        int i = 0;
        while (input->next()) {
            eckit::Log::info() << "============> " << what << ": " << (++i) << std::endl;


            // Set representation and grid
            const mir::repres::Representation* outputRepresentation = getOutputRepresentation(parametrisation, local);
            ASSERT(outputRepresentation);

            atlas::Grid outputGrid = getOutputGrid(parametrisation, *outputRepresentation);
            ASSERT(outputGrid);

            mir::util::MIRStatistics statistics;
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

            eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: Atlas/Trans configuration type: " << transConfig.getString("type", "(default)") << std::endl;
            eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: Atlas unstructured grid: " << (atlas::grid::UnstructuredGrid(outputGrid) ? "yes" : "no") << std::endl;


            // Trans inverse transform
            if (vod2uv) {
                ASSERT(field.dimensions() == 2);

                const std::vector<double>& spectra_vo = field.values(0);
                const std::vector<double>& spectra_d  = field.values(1);
                ASSERT(spectra_vo.size() == N * 2);
                ASSERT(spectra_vo.size() == spectra_d.size());

                const size_t Ngp = outputGrid.size();
                std::vector<double> output(Ngp * 2);

                trans.invtrans(/* nb_vordiv_fields = */ 1, spectra_vo.data(), spectra_d.data(), output.data());

                // copy u/v result, forcing paramId
                const eckit::Configuration& config = mir::LibMir::instance().configuration();

                std::vector<double> result;
                result.assign(output.begin(), output.begin() + int(Ngp));
                field.update(result, 0);
                field.metadata(0, "paramId", config.getLong("parameter-id-u", 131));

                result.assign(output.begin() + int(Ngp), output.end());
                field.update(result, 1);
                field.metadata(1, "paramId", config.getLong("parameter-id-v", 132));

            } else {
                for (size_t d = 0; d < field.dimensions(); ++d) {

                    const std::vector<double>& spectra = field.values(d);
                    ASSERT(spectra.size() == N * 2);

                    std::vector<double> result(outputGrid.size(), 0);

                    trans.invtrans(/* nb_scalar_fields = */ 1, spectra.data(), result.data());

                    field.update(result, d);
                }
            }

            // set field representation
            field.representation(outputRepresentation);
            if (validate) {
                field.validate();
            }

            // save
            output->save(parametrisation, ctx);
        }

        eckit::Log::info() << eckit::Plural(i, what) << " in " << eckit::Seconds(timer.elapsed())
                           << ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;
    }
}


int main(int argc, char** argv) {
    trans_use_mpi(false);
    trans_init();

    MIRSpectralTransform tool(argc, argv);

    int r = tool.start();

    trans_finalize();
    return r;
}

