/*
 * (C) Copyright 1996-2016 ECMWF.
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
#include "eckit/log/BigNum.h"
#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/types/FloatCompare.h"
#include "atlas/array/IndexView.h"
#include "atlas/functionspace/FunctionSpace.h"
#include "atlas/grid.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/actions/BuildConvexHull3D.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/util/Constants.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/repres/Gridded.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/tools/MIRTool.h"


class MIRIntegrate : public mir::tools::MIRTool {

    // -- Overridden methods

    void execute(const eckit::option::CmdArgs&);

    void usage(const std::string &tool) const;

    int minimumPositionalArguments() const {
        return 1;
    }

public:

    // -- Contructors

    MIRIntegrate(int argc, char **argv) : mir::tools::MIRTool(argc, argv) {}

};


void MIRIntegrate::usage(const std::string &tool) const {
    eckit::Log::info()
            << "\n" << "Usage: " << tool << " file.grib"
            << std::endl;
}


void MIRIntegrate::execute(const eckit::option::CmdArgs& args) {

    using atlas::interpolation::element::Triag3D;
    using atlas::interpolation::element::Quad3D;
    using atlas::util::Constants;

//    options_t options;
//    options.push_back(new eckit::option::SimpleOption<size_t>("buckets", "Bucket count for computing entropy (default 65536)"));

    mir::input::GribFileInput file(args(0));
    mir::input::MIRInput &input = file;

    size_t n = 0;
    while ( file.next() ) {

        ++n;

        mir::data::MIRField field(input.field());

        const std::vector<double>& values = field.values(0);

        ASSERT(!field.hasMissing());

        const mir::repres::Representation* rep = field.representation();

        ASSERT(rep);
        // ASSERT(rep->domain().isGlobal());

#if 0
        eckit::ScopedPtr<atlas::Grid> grid( rep->atlasGrid() );

        atlas::Mesh& mesh = grid->mesh();

        atlas::mesh::actions::BuildXYZField()(mesh);
        atlas::mesh::actions::BuildConvexHull3D builder;
        builder(mesh);

        atlas::Nodes& nodes  = mesh.nodes();
        atlas::array::ArrayView<double, 2> coords  ( nodes.field( "xyz" ));

        atlas::FunctionSpace& triags = mesh.function_space( "triags" );
        atlas::array::IndexView<int, 2> triag_nodes ( triags.field( "nodes" ) );

        atlas::FunctionSpace& quads = mesh.function_space( "quads" );
        atlas::array::IndexView<int, 2> quads_nodes ( quads.field( "nodes" ) );

        size_t nb_triags = triags.shape(0);
        size_t nb_quads  = quads.shape(0);

        double result = 0.;

        for(size_t e = 0; e < nb_triags; ++e) {
            size_t idx [3];
            for(size_t n = 0; n<3; ++n)
                idx[n] = triag_nodes(e,n);

            Triag3D triag(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data());

            const double area = triag.area();

            /// TODO add check for virtuals

            for(size_t i = 0; i<3; ++i)
                result += area * oneThird * values[idx[i]];
        }

        for(size_t e = 0; e < nb_quads; ++e) {
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

        const atlas::grid::StructuredGrid structured(rep->atlasGrid());
        ASSERT(structured);

        size_t i = 0;
        for(size_t jlat = 0; jlat < structured.ny(); ++jlat) {

            size_t pts_on_latitude = structured.nx(jlat);

            const double lat = structured.y(jlat);

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


int main(int argc, char **argv) {
    MIRIntegrate tool(argc, argv);
    return tool.start();
}


