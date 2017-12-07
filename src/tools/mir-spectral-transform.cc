/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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

#include "atlas/array.h"
#include "atlas/grid.h"
#include "atlas/trans/Trans.h"
#include "atlas/trans/local/FourierTransforms.h"
#include "atlas/trans/local/LegendrePolynomials.h"
#include "atlas/trans/local/LegendreTransforms.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/output/MIROutput.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/Representation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/repres/sh/SphericalHarmonics.h"
#include "mir/tools/MIRTool.h"
#include "mir/util/MIRStatistics.h"


//-----------------------------------------------------------------------------


struct Environment {
    Environment() {
        trans_use_mpi(false);
        trans_init();
    }
    ~Environment() {
        trans_finalize();
    }
};


void compute_legendre(
        const size_t trc,                            // truncation (in)
        double& lat,                                 // latitude in radians (in)
        atlas::array::ArrayView<double,1>& zlfpol )  // values of associated Legendre functions, size (trc+1)*trc/2 (out)
{
    atlas::trans::compute_legendre_polynomials(trc,lat,zlfpol.data());
}


void legendre_transform(
        const size_t trc,                             // truncation (in)
        const size_t trcFT,                           // truncation for Fourier transformation (in)
        atlas::array::ArrayView<double,1>& rlegReal,  // values of associated Legendre functions, size (trc+1)*trc/2 (out)
        atlas::array::ArrayView<double,1>& rlegImag,  // values of associated Legendre functions, size (trc+1)*trc/2 (out)
        atlas::array::ArrayView<double,1>& zlfpol,    // values of associated Legendre functions, size (trc+1)*trc/2 (out)
        double rspecg[] )                             // spectral data, size (trc+1)*trc (in)
{
    atlas::trans::invtrans_legendre( trc, trcFT, zlfpol.data(), rspecg, rlegReal.data(), rlegImag.data() );
}


double fourier_transform(
        const size_t trcFT,
        atlas::array::ArrayView<double,1>& rlegReal,  // values of associated Legendre functions, size (trc+1)*trc/2 (out)
        atlas::array::ArrayView<double,1>& rlegImag,  // values of associated Legendre functions, size (trc+1)*trc/2 (out)
        double lon )                                  // radians
{
    return atlas::trans::invtrans_fourier( trcFT, rlegReal.data(), rlegImag.data(), lon );
}


//-----------------------------------------------------------------------------
// Routine to compute the spectral transform by using a local Fourier transformation
// for a single point
//
// Author:
// Andreas Mueller *ECMWF*
//
double spectral_transform_point(
        const size_t trc,                  // truncation (in)
        const size_t trcFT,                // truncation for Fourier transformation (in)
        double lon,                        // longitude in radians (in)
        double lat,                        // latitude in radians (in)
        double rspecg[] )                  // spectral data, size (trc+1)*trc (in)
{
    using eckit::testing::make_view;

    size_t N = (trc+2)*(trc+1)/2;

    atlas::array::ArrayT<double> zlfpol_(N);
    atlas::array::ArrayView<double,1> zlfpol = make_view<double,1>(zlfpol_);

    atlas::array::ArrayT<double> rlegReal_(trcFT+1);
    atlas::array::ArrayView<double,1> rlegReal = make_view<double,1>(rlegReal_);

    atlas::array::ArrayT<double> rlegImag_(trcFT+1);
    atlas::array::ArrayView<double,1> rlegImag = make_view<double,1>(rlegImag_);

    // Legendre transform:
    compute_legendre(trc, lat, zlfpol);
    legendre_transform(trc, trcFT, rlegReal, rlegImag, zlfpol, rspecg);

    // Fourier transform:
    return fourier_transform(trcFT, rlegReal, rlegImag, lon);
}


//-----------------------------------------------------------------------------
// Routine to compute the spectral transform by using a local Fourier transformation
// for a grid (same latitude for all longitudes, allows to compute Legendre functions
// once for all longitudes)
//
// Author:
// Andreas Mueller *ECMWF*
//
void spectral_transform_grid(
        const size_t trc,     // truncation (in)
        const size_t trcFT,   // truncation for Fourier transformation (in)
        atlas::Grid grid,     // call with something like Grid("O32")
        double rspecg[],      // spectral data, size (trc+1)*trc (in)
        double rgp[],         // resulting grid point data (out)
        bool pointwise)       // use point function for unstructured mesh for testing purposes
{
    using eckit::testing::make_view;

    size_t N = (trc+2)*(trc+1)/2;

    atlas::array::ArrayT<double> zlfpol_(N);
    atlas::array::ArrayView<double,1> zlfpol = make_view<double,1>(zlfpol_);

    atlas::array::ArrayT<double> rlegReal_(trcFT+1);
    atlas::array::ArrayView<double,1> rlegReal = make_view<double,1>(rlegReal_);

    atlas::array::ArrayT<double> rlegImag_(trcFT+1);
    atlas::array::ArrayView<double,1> rlegImag = make_view<double,1>(rlegImag_);

    for( size_t jm=0; jm<grid.size(); jm++) rgp[jm] = 0.;

    if( atlas::grid::StructuredGrid(grid) ) {
        atlas::grid::StructuredGrid g(grid);
        int idx = 0;
        for( size_t j=0; j<g.ny(); ++j ) {
            double lat = g.y(j) * atlas::util::Constants::degreesToRadians() ;

            // Legendre transform:
            compute_legendre(trc, lat, zlfpol);
            legendre_transform(trc, trcFT, rlegReal, rlegImag, zlfpol, rspecg);

            for( size_t i=0; i<g.nx(j); ++i ) {
                double lon = g.x(i,j) * atlas::util::Constants::degreesToRadians();
                // Fourier transform:
                rgp[idx++] = fourier_transform(trcFT, rlegReal, rlegImag, lon);
            }
        }
    } else {
        int idx = 0;
        for( atlas::PointXY p: grid.xy()) {
            double lon = p.x() * atlas::util::Constants::degreesToRadians();
            double lat = p.y() * atlas::util::Constants::degreesToRadians();
            if( pointwise ) {
                // alternative for testing: use spectral_transform_point function:
                rgp[idx++] = spectral_transform_point(trc, trcFT, lon, lat, rspecg);
            } else {
                // Legendre transform:
                compute_legendre(trc, lat, zlfpol);
                legendre_transform(trc, trcFT, rlegReal, rlegImag, zlfpol, rspecg);

                // Fourier transform:
                rgp[idx++] = fourier_transform(trcFT, rlegReal, rlegImag, lon);
            }
        }
    }
}


//-----------------------------------------------------------------------------


class MIRSpectralTransform : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 2;
    }

public:

    MIRSpectralTransform(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new Separator("Output description (mandatory one option)"));
        options_.push_back(new VectorOption<double>          ("grid", "Regular latitude/longitude grid (regular_ll) increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<std::string>     ("gridname", "Interpolate to given grid name"));
        options_.push_back(new SimpleOption<eckit::PathName> ("griddef", "Path to file containing latitude/longitude pairs"));

        options_.push_back(new Separator("Other"));
        options_.push_back(new VectorOption<double> ("area", "Regular latitude/longitude grid (regular_ll) bounding box (North/West/South/East)", 4));
        options_.push_back(new SimpleOption<size_t> ("truncation", "Use up to spectral truncation, up to input (default no truncation)"));
        options_.push_back(new SimpleOption<bool>   ("pointwise", "Apply point-wise inverse transform"));
//        options_.push_back(new SimpleOption<bool>   ("vod2uv", "Input is vorticity and divergence (vo/d), convert to Cartesian components (gridded u/v or spectral U/V)"));
        options_.push_back(new SimpleOption<bool>   ("unstructured", "Force unstructured transform"));
        options_.push_back(new SimpleOption<bool>   ("caching", "Caching (default 1)"));
    }

};


void MIRSpectralTransform::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" "Usage: " << tool << " --grid=WE/SN|--gridname=<namedgrid>|--griddef=<path> [--key1=value [--key2=value [...]]] input.grib output.grib"
            "\n" "Examples: "
            "\n" "  % " << tool << " go-for-it"
            << std::endl;
}



enum trans_call_t { invtrans_scalar, invtrans_vod2uv };
enum grid_mode_t { grid_structured, grid_unstructured };


void MIRSpectralTransform::execute(const eckit::option::CmdArgs& args) {
    eckit::ResourceUsage usage("MIRSpectralTransform");


    // Setup options

    const mir::param::ConfigurationWrapper parametrisation(args);

    std::string griddef;
    std::string gridname;
    std::vector<double> grid;
    std::vector<double> area;

    if (args.has("grid") + args.has("gridname") + args.has("griddef") != 1) {
        throw eckit::UserError("Output description is required: either 'grid', 'gridname' or 'griddef'");
    }

    if (args.get("grid", grid))         { ASSERT(grid.size() == 2); }
    if (args.get("area", area))         { ASSERT(area.size() == 4); }
    if (args.get("griddef", griddef))   { ASSERT(!griddef.empty()); }
    if (args.get("gridname", gridname)) { ASSERT(!gridname.empty()); }

    const bool pointwise = args.has("pointwise");


    // Setup output: representation
    mir::repres::Representation* outputRepresentation = 0;

    eckit::ScopedPtr< mir::util::Increments > increments;
    eckit::ScopedPtr< mir::util::BoundingBox > boundingBox;
    if (args.has("grid")) {

        increments.reset(new mir::util::Increments(grid[0], grid[1]));
        ASSERT(increments);

        if (args.has("area")) {
            boundingBox.reset(new mir::util::BoundingBox(area[0],  area[1], area[2], area[3]));
        } else {
            // temporary: do not shift lat/lon so transform comparisons are 'fairer'
            boundingBox.reset(new mir::util::BoundingBox());
            increments->globaliseBoundingBox(*boundingBox, false, false);
        }
        ASSERT(boundingBox);

        outputRepresentation = new mir::repres::latlon::RegularLL(*boundingBox, *increments);

    } else if (args.has("griddef")) {

        outputRepresentation = new mir::repres::other::UnstructuredGrid(eckit::PathName(griddef));

    } else if (args.has("gridname")) {

        const mir::namedgrids::NamedGrid& ng = mir::namedgrids::NamedGrid::lookup(gridname);
        outputRepresentation = const_cast< mir::repres::Representation* >(ng.representation());

    }
    ASSERT(outputRepresentation != 0);


    // Setup output: Atlas grid
    atlas::Grid outputGrid;

    if (args.has("unstructured") || args.has("area") || args.has("griddef")) {
        eckit::Log::info() << "Atlas grid: unstructured" << std::endl;

        std::vector< atlas::PointXY >* coordinates = new std::vector< atlas::PointXY >;

        eckit::ScopedPtr< mir::repres::Iterator > it(outputRepresentation->iterator());
        coordinates->reserve(outputRepresentation->count());

        while (it->next()) {
            const mir::repres::Iterator::point_2d_t& p(*(*it));
            coordinates->push_back(atlas::PointXY(p[1], p[0]));
        }

        outputGrid = atlas::grid::UnstructuredGrid(coordinates);

    } else {

        eckit::Log::info() << "Atlas grid: structured" << std::endl;
        outputGrid = outputRepresentation->atlasGrid();

    }
    ASSERT(outputGrid);


    // Setup output: file
    eckit::ScopedPtr<mir::output::MIROutput> output(mir::output::MIROutputFactory::build(args(1), parametrisation));
    ASSERT(output);


    {
        eckit::Timer timer("Total time");

        bool vod2uv = false;
        args.get("vod2uv", vod2uv);

        const std::string what(vod2uv ? "vo/d field pair" : "field");

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

                int i = 0;
        while (input->next()) {
            eckit::Log::info() << "============> " << what << ": " << (++i) << std::endl;

            mir::util::MIRStatistics statistics;
            mir::context::Context ctx(*input, statistics);

            // Keep a pointer on the original representation, as the one in the field will
            // be changed in the loop
            mir::data::MIRField& field = ctx.field();
            mir::repres::RepresentationHandle inputRepresentation(field.representation());

            for (size_t i = 0; i < field.dimensions(); i++) {

                size_t T = inputRepresentation->truncation();
                ASSERT(T > 0);

                size_t Tin = 0;
                args.get("truncation", Tin);
                Tin = Tin ? std::min(Tin, T) : T;

                size_t N = mir::repres::sh::SphericalHarmonics::number_of_complex_coefficients(T);
                ASSERT(N > 0);

                std::vector<double>& rspecg = const_cast<std::vector<double>&>( field.values(i) );
                std::vector<double> rgp(outputGrid.size(), 0);

                spectral_transform_grid(T, Tin, outputGrid, rspecg.data(), rgp.data(), pointwise);

                field.representation(outputRepresentation);
                field.update(rgp, i);

                // consider not validating
                field.validate();
            }

            output->save(parametrisation, ctx);
        }

        eckit::Log::info() << eckit::Plural(i, what) << " in " << eckit::Seconds(timer.elapsed()) <<
                              ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;
    }
}


int main(int argc, char **argv) {
    MIRSpectralTransform tool(argc, argv);
    return tool.start();
}

