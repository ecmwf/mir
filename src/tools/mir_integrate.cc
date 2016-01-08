/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @date   Jul 2015

#include <cmath>

#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/runtime/Tool.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/log/BigNum.h"

#include "atlas/Parameters.h"
#include "atlas/Grid.h"
#include "atlas/Mesh.h"
#include "atlas/FunctionSpace.h"
#include "atlas/util/IndexView.h"
#include "atlas/actions/BuildXYZField.h"
#include "atlas/actions/BuildConvexHull3D.h"
#include "atlas/geometry/Triag3D.h"
#include "atlas/geometry/Quad3D.h"
#include "atlas/grids/ReducedGrid.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/param/MIRArgs.h"
#include "mir/param/option/SimpleOption.h"
#include "mir/repres/Representation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Gridded.h"

using atlas::Constants;
using atlas::grids::ReducedGrid;
using atlas::geometry::Triag3D;
using atlas::geometry::Quad3D;

using mir::param::option::Option;
using mir::param::option::SimpleOption;

using namespace mir;

class MIRIntegrate : public eckit::Tool {

    virtual void run();

    static void usage(const std::string &tool);


  public:
    MIRIntegrate(int argc, char **argv) :
        eckit::Tool(argc, argv) {
    }
};

void MIRIntegrate::usage(const std::string &tool) {

    eckit::Log::info()
            << std::endl << "Usage: " << tool << " file.grib" << std::endl;

    ::exit(1);
}

static const double oneThird  = 1./ 3.;
static const double oneFourth = 1./ 4.;

void MIRIntegrate::run() {

    using eckit::FloatApproxCompare;

    std::vector<const Option *> options;

//     options.push_back(new SimpleOption<size_t>("buckets", "Bucket count for computing entropy (default 65536)"));

    mir::param::MIRArgs args(&usage, 1, options);

    mir::input::GribFileInput file(args.args(0));

    mir::input::MIRInput &input = file;

    const mir::param::MIRParametrisation &metadata = input.parametrisation();

    size_t n = 0;
    while ( file.next() ) {

        ++n;

        eckit::ScopedPtr<mir::data::MIRField> field(input.field());

        const std::vector<double>& values = field->values(0);

        ASSERT(!field->hasMissing());

        const repres::Representation* rep = field->representation();

        ASSERT(rep);
        // ASSERT(rep->globalDomain());

#if 0
        eckit::ScopedPtr<atlas::Grid> grid( rep->atlasGrid() );

        atlas::Mesh& mesh = grid->mesh();

        atlas::actions::BuildXYZField()(mesh);
        atlas::actions::BuildConvexHull3D builder;
        builder(mesh);

        atlas::Nodes& nodes  = mesh.nodes();
        atlas::ArrayView<double, 2> coords  ( nodes.field( "xyz" ));

        atlas::FunctionSpace& triags = mesh.function_space( "triags" );
        atlas::IndexView<int, 2> triag_nodes ( triags.field( "nodes" ) );

        atlas::FunctionSpace& quads = mesh.function_space( "quads" );
        atlas::IndexView<int, 2> quads_nodes ( quads.field( "nodes" ) );

        size_t nb_triags = triags.shape(0);
        size_t nb_quads  = quads.shape(0);

        double result = 0.;

        for(size_t e = 0; e < nb_triags; ++e)
        {
            size_t idx [3];
            for(size_t n = 0; n<3; ++n)
              idx[n] = triag_nodes(e,n);

            Triag3D triag(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data());

            const double area = triag.area();

            /// TODO add check for virtuals

            for(size_t i = 0; i<3; ++i)
                result += area * oneThird * values[idx[i]];
        }

        for(size_t e = 0; e < nb_quads; ++e)
        {
            size_t idx [4];
            for(size_t n = 0; n<4; ++n)
              idx[n] = quads_nodes(e,n);

            Quad3D quad(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data(), coords[idx[3]].data());

            const double area = quad.area();

            /// TODO add check for virtuals

            for(size_t i = 0; i<4; ++i)
                result += area * oneFourth * values[idx[i]];
        }
#else
        double result = 0;
        double weights = 0;

        eckit::ScopedPtr<atlas::Grid> grid( rep->atlasGrid() );

        const atlas::grids::ReducedGrid *reduced = dynamic_cast<const atlas::grids::ReducedGrid*>(grid.get());

        ASSERT(reduced);

        size_t i = 0;
        for(size_t jlat = 0; jlat < reduced->nlat(); ++jlat) {

            size_t pts_on_latitude = reduced->nlon(jlat);

            const double lat = reduced->lat(jlat);

            for(size_t jlon = 0; jlon < pts_on_latitude; ++jlon) {
                const double w = cos( lat * Constants::degreesToRadians() ) / pts_on_latitude;
                result  += w * values[i++];
                weights += w;
            }
        }

        ASSERT(i == values.size());

        result /= weights;

#endif


        eckit::Log::info() << "Integral " << result << std::endl;

    }
}


int main( int argc, char **argv ) {
    MIRIntegrate tool(argc, argv);
    return tool.execute();
}

