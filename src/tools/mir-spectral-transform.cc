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
#include "eckit/option/CmdArgs.h"
#include "eckit/option/FactoryOption.h"
#include "eckit/option/Option.h"
#include "eckit/option/Separator.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/option/VectorOption.h"

#include "atlas/grid/Grid.h"

#include "mir/api/MIRJob.h"
#include "mir/input/GeoPointsFileInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/VectorInput.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/output/MIROutput.h"
#include "mir/packing/Packer.h"
#include "mir/param/ConfigurationWrapper.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/tools/MIRTool.h"


class MIRSpectralTransform : public mir::tools::MIRTool {
private:

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string& tool) const;

    int minimumPositionalArguments() const {
        return 2;
    }

    void process(
            mir::api::MIRJob&,
            mir::input::MIRInput&,
            mir::output::MIROutput&,
            const atlas::Grid&,
            const std::string& );

public:

    MIRSpectralTransform(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {
        using namespace eckit::option;

        options_.push_back(new Separator("Output description (mandatory one option)"));
        options_.push_back(new VectorOption<double>          ("grid", "Regular latitude/longitude grid (regular_ll) increments (West-East/South-North)", 2));
        options_.push_back(new SimpleOption<std::string>     ("gridname", "Interpolate to given grid name"));
        options_.push_back(new SimpleOption<eckit::PathName> ("griddef", "Path to file containing latitude/longitude pairs"));

        options_.push_back(new Separator("Other"));
        options_.push_back(new VectorOption<double> ("area", "Regular latitude/longitude grid (regular_ll) bounding box (North/West/South/East)", 4));
        options_.push_back(new SimpleOption<bool>   ("vod2uv", "Input is vorticity and divergence (vo/d), convert to Cartesian components (gridded u/v or spectral U/V)"));
        options_.push_back(new SimpleOption<bool>   ("unstructured", "Force unstructured transform"));
        options_.push_back(new SimpleOption<bool>   ("caching", "Caching (default 1)"));
//        options_.push_back(new SimpleOption<size_t>                ("accuracy", "GRIB number of bits per value"));
//        options_.push_back(new FactoryOption<mir::packing::Packer> ("packing",  "GRIB packing method"));
//        options_.push_back(new SimpleOption<size_t>                ("edition",  "GRIB edition number"));
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
    mir::api::MIRJob job;
    args.configure(job);

    const mir::param::ConfigurationWrapper args_wrap(args);
    bool vod2uv = false;
    args_wrap.userParametrisation().get("vod2uv", vod2uv);

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


    // Setup representation
    mir::repres::Representation* repres = 0;

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

        repres = new mir::repres::latlon::RegularLL(*boundingBox, *increments);

    } else if (args.has("griddef")) {

        repres = new mir::repres::other::UnstructuredGrid(eckit::PathName(griddef));

    } else if (args.has("gridname")) {

        const mir::namedgrids::NamedGrid& ng = mir::namedgrids::NamedGrid::lookup(gridname);
        repres = const_cast< mir::repres::Representation* >(ng.representation());

    }
    ASSERT(repres != 0);


    // Setup Atlas grid
    atlas::Grid outputGrid;

    if (args.has("unstructured") || args.has("area") || args.has("griddef")) {
        eckit::Log::info() << "Atlas grid: unstructured" << std::endl;

        std::vector< atlas::PointXY >* coordinates = new std::vector< atlas::PointXY >;

        eckit::ScopedPtr< mir::repres::Iterator > it(repres->iterator());
        coordinates->reserve(repres->count());

        while (it->next()) {
            const mir::repres::Iterator::point_2d_t& p(*(*it));
            coordinates->push_back(atlas::PointXY(p[1], p[0]));
        }

        outputGrid = atlas::grid::UnstructuredGrid(coordinates);

    } else {

        eckit::Log::info() << "Atlas grid: structured" << std::endl;
        outputGrid = repres->atlasGrid();

    }
    ASSERT(outputGrid);


    // Setup output
    eckit::ScopedPtr<mir::output::MIROutput> output(mir::output::MIROutputFactory::build(args(1), args_wrap));
    ASSERT(output);


    // Setup input, and process
    if (vod2uv) {

        mir::input::GribFileInput vo(args(0), 0, 2);
        mir::input::GribFileInput d(args(0), 1, 2);
        mir::input::VectorInput input(vo, d);

        process(job, input, *output, outputGrid, "vo/d field pair");
        return;
    }

    mir::input::GribFileInput input(args(0));
    process(job, input, *output, outputGrid, "field");
}


void MIRSpectralTransform::process(
        mir::api::MIRJob& job,
        mir::input::MIRInput& input,
        mir::output::MIROutput& output,
        const atlas::Grid& grid,
        const std::string& what ) {
    eckit::Timer timer("Total time");

    size_t i = 0;
    while (input.next()) {
        eckit::Log::info() << "============> " << what << ": " << (++i) << std::endl;
        job.execute(input, output);
    }

    eckit::Log::info() << eckit::Plural(i, what) << " in " << eckit::Seconds(timer.elapsed()) <<
                       ", rate: " << double(i) / double(timer.elapsed()) << " " << what << "/s" << std::endl;
}


int main(int argc, char **argv) {
    MIRSpectralTransform tool(argc, argv);
    return tool.start();
}

