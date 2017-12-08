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
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"


class MIRSpectralTransform : public mir::tools::MIRTool {
private:
    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const { return 2; }

public:
    MIRSpectralTransform(int argc, char** argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new Separator("Output description (mandatory one option)"));
        options_.push_back(new VectorOption<double>          ("grid", "Regular latitude/longitude grid (regular_ll) increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<std::string>     ("gridname", "Interpolate to given grid name"));
        options_.push_back(new SimpleOption<eckit::PathName> ("griddef", "Path to file containing latitude/longitude pairs"));

        options_.push_back(new Separator("Other"));
        options_.push_back(new VectorOption<double> ("area", "Regular latitude/longitude grid (regular_ll) bounding box (North/West/South/East)", 4));
//        options_.push_back(new SimpleOption<bool>   ("vod2uv", "Input is vorticity and divergence (vo/d), convert to Cartesian components (gridded u/v or spectral U/V)"));
        options_.push_back(new SimpleOption<bool>   ("local", "Force local transform"));
        options_.push_back(new SimpleOption<bool>   ("caching", "Caching (default 1)"));
        options_.push_back(new SimpleOption<bool>   ("validate", "Validate results"));
    }
};


void MIRSpectralTransform::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " --grid=WE/SN|--gridname=<namedgrid>|--griddef=<path> [--key1=value [--key2=value [...]]] input.grib output.grib"
            "\n" "Examples: "
            "\n" "  % " << tool << " go-for-it"
            << std::endl;
}


const mir::repres::Representation* getOutputRepresentation(const mir::param::MIRParametrisation& parametrisation) {
    const mir::repres::Representation* representation = 0;

    std::string griddef;
    std::string gridname;
    std::vector<double> grid;
    std::vector<double> area;

    eckit::ScopedPtr< mir::util::Increments > increments;
    eckit::ScopedPtr< mir::util::BoundingBox > boundingBox;
    if (parametrisation.get("grid", grid)) {
        ASSERT(grid.size() == 2);

        increments.reset(new mir::util::Increments(grid[0], grid[1]));
        ASSERT(increments);

        if (parametrisation.get("area", area)) {
            ASSERT(area.size() == 4);
            boundingBox.reset(new mir::util::BoundingBox(area[0], area[1], area[2], area[3]));
        } else {
            // temporary: do not shift lat/lon
            boundingBox.reset(new mir::util::BoundingBox());
            increments->globaliseBoundingBox(*boundingBox, false, false);
        }
        ASSERT(boundingBox);

        representation = new mir::repres::latlon::RegularLL(*boundingBox, *increments);

    } else if (parametrisation.get("griddef", griddef)) {

        ASSERT(!griddef.empty());
        representation = new mir::repres::other::UnstructuredGrid(eckit::PathName(griddef));

    } else if (parametrisation.get("gridname", gridname)) {

        ASSERT(!gridname.empty());
        representation = mir::namedgrids::NamedGrid::lookup(gridname).representation();

    }

    ASSERT(representation);
    return representation;
}


atlas::Grid getOutputGrid(const mir::param::MIRParametrisation& parametrisation, const mir::repres::Representation& representation) {
    atlas::Grid outputGrid;

    if (parametrisation.has("griddef")) {
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

    local = local || args.has("area") || args.has("griddef");
    eckit::Log::debug<mir::LibMir>() << "MIRSpectralTransform: Trans configuration type=" << (local ? "'local'" : "(default)") << std::endl;


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
            const mir::repres::Representation* outputRepresentation = getOutputRepresentation(parametrisation);
            ASSERT(outputRepresentation);

            atlas::Grid outputGrid = getOutputGrid(parametrisation, *outputRepresentation);
            ASSERT(outputGrid);

            mir::util::MIRStatistics statistics;
            mir::context::Context ctx(*input, statistics);
            mir::data::MIRField& field = ctx.field();

            for (size_t d = 0; d < field.dimensions(); ++d) {

                size_t T = field.representation()->truncation();
                ASSERT(T > 0);

                size_t N = mir::repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
                ASSERT(N > 0);

                // spectral data
                const std::vector<double>& spectra = field.values(d);
                ASSERT(spectra.size() == N * 2);

                // grid point data
                std::vector<double> result(outputGrid.size(), 0);

                // set trans and perform inverse transform
                atlas::util::Config transConfig;
                if (local) {
                    transConfig.set("type", "local");
                }

                atlas::trans::Trans trans(outputGrid, int(T), transConfig);
                trans.invtrans(/* nb_scalar_fields = */ 1, spectra.data(), result.data());

                // set field grid point data
                field.update(result, d);
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

