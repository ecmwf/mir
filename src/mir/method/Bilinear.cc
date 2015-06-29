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
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/method/Bilinear.h"

#include <string>
#include <algorithm>

#include "atlas/Mesh.h"
#include "atlas/FunctionSpace.h"
#include "atlas/Field.h"
#include "atlas/util/ArrayView.h"
#include "atlas/grids/ReducedGaussianGrid.h"

#include "eckit/log/Log.h"


namespace mir {
namespace method {


namespace {


bool eq(const double& a, const double& b) {
    // @todo use the one in eckit once it stops giving you gip
    return fabs(a-b) < 10e-10;
}


void left_right_lon_indexes(const double& in, atlas::ArrayView<double,2>& data, size_t start, size_t end, size_t& left, size_t& right) {
    using eckit::geometry::LON;

    double right_lon = 360.0;
    double left_lon = 0.0;

    right = start; // take the first if there's a wrap
    left = start;

    for (unsigned int i = start; i < end; i++) {
        const double& val = data[i].data()[LON];

        if (val < in || eq(val, in)) {
            left_lon = val;
            left = i;
        } else {
            if (val < right_lon) {
                right_lon = val;
                right = i;
            }
        }
    }
}


}  // (utilities namespace)


Bilinear::Bilinear(const param::MIRParametrisation& param) :
    MethodWeighted(param) {
}


Bilinear::~Bilinear() {
}

const char* Bilinear::name() const {
    return  "bilinear";
}

void Bilinear::hash(eckit::MD5 &md5) const {
    MethodWeighted::hash(md5);
}

void Bilinear::assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const {

    using eckit::geometry::LON;
    using eckit::geometry::LAT;

    eckit::Log::info() << "Bilinear::assemble " << *this << std::endl;

    const atlas::Mesh& i_mesh = in.mesh();
    const atlas::Mesh& o_mesh = out.mesh();

    atlas::FunctionSpace& inodes = i_mesh.function_space( "nodes" );
    atlas::FunctionSpace& onodes = o_mesh.function_space( "nodes" );

    atlas::ArrayView<double,2> icoords ( inodes.field( "lonlat" ) );
    atlas::ArrayView<double,2> ocoords ( onodes.field( "lonlat" ) );

    // ReducedGrid involves all grids that can be represented with latitudes and npts_per_lat
    const atlas::grids::ReducedGrid* igg = dynamic_cast<const atlas::grids::ReducedGrid*>(&in);

    /// @todo we only handle these at the moment
    if (!igg)
        throw eckit::UserError("Bilinear currently only supports Reduced Grids as input");

    // get the longitudes from
    const std::vector<int>& lons = igg->npts_per_lat();

    std::vector< WeightMatrix::Triplet > weights_triplets; /* structure to fill-in sparse matrix */

    // determing the number of output points required
    const size_t out_npts = onodes.shape(0);
    weights_triplets.reserve( out_npts );

    for (unsigned int i = 0; i < out_npts; i++) {
        // get the lat, lon of the output data point required
        double lat = ocoords(i,LAT);
        double lon = ocoords(i,LON);

        // these will hold indices in the input vector of the start of the upper and lower latitudes
        size_t top_i = 0;
        size_t bot_i = 0;


        // we will need the number of points on the top and bottom latitudes later. store them
        size_t top_n, bot_n;

        for (unsigned int n = 0; n < lons.size() - 1; n++) {
            top_i = bot_i;
            bot_i += lons[n];

            top_n = lons[n];
            if ( (n + 1 ) == lons.size())
                bot_n = 0;
            else
                bot_n = lons[n+1];

            double top_lat = icoords(top_i,LAT);

            double bot_lat = icoords(bot_i,LAT);
            ASSERT(top_lat != bot_lat);

            // check output point is on or below the hi latitude
            if (bot_lat < lat && (top_lat > lat || eq(top_lat, lat))) {
                ASSERT(top_lat > lat || eq(top_lat, lat));
                ASSERT(bot_lat < lat);
                ASSERT(!eq(bot_lat, lat));

                break;
            }
        }

        double top_lat = icoords(top_i,LAT);
        double bot_lat = icoords(bot_i,LAT);
        ASSERT(top_lat > lat || eq(top_lat, lat));
        ASSERT(bot_lat < lat);
        ASSERT(!eq(bot_lat, lat));


        // now get indeces to the left and right points on each of the data sectors
        // on the upper longitude
        size_t top_i_lft, top_i_rgt;

        left_right_lon_indexes(lon, icoords, top_i, top_i + top_n, top_i_lft, top_i_rgt);
        //left_right_lon_indexes(lon, icoords,  hi_data, tl, tr);
        ASSERT(top_i_lft >= top_i);
        ASSERT(top_i_lft < bot_i);
        ASSERT(top_i_rgt >= top_i);
        ASSERT(top_i_rgt < bot_i);
        ASSERT(top_i_rgt != top_i_lft);

        // check the data is the same
        ASSERT(eq(icoords(top_i_lft,LAT),  icoords(top_i_rgt,LAT)));

        // now get indeces to the left and right points on each of the data sectors
        // on the lower longitude
        size_t bot_i_lft, bot_i_rgt;

        size_t bot_i_end = bot_i + bot_n;

        left_right_lon_indexes(lon, icoords, bot_i, bot_i_end , bot_i_lft, bot_i_rgt);
        ASSERT(bot_i_lft >= bot_i);
        ASSERT(bot_i_lft < bot_i_end);
        ASSERT(bot_i_rgt >= bot_i);
        ASSERT(bot_i_rgt < bot_i_end);
        ASSERT(bot_i_rgt != bot_i_lft);
        ASSERT(eq(icoords(bot_i_lft,LAT),  icoords(bot_i_rgt,LAT)));

        // we now have the indices of tl, tr, bl, br points around the output point

        double tl_lon  = icoords(top_i_lft,LON);
        double tr_lon  = icoords(top_i_rgt,LON);
        double bl_lon  = icoords(bot_i_lft,LON);
        double br_lon  = icoords(bot_i_rgt,LON);

        // calculate the weights
        double w1 = (tl_lon - lon) / (tl_lon - tr_lon);
        double w2 = 1.0 - w1;
        double w3 = (bl_lon - lon) / (bl_lon - br_lon);
        double w4 = 1.0 - w3;

        // top and bottom midpoint weights
        double wt = (lat - bot_lat) / (top_lat - bot_lat);
        double wb = 1.0 - wt;

        // weights for the tl, tr, bl, br points
        double w_tl =  w2 * wt;
        double w_tr =  w1 * wt;
        double w_bl =  w4 * wb;
        double w_br =  w3 * wb;

        weights_triplets.push_back( WeightMatrix::Triplet( i, bot_i_rgt, w_br ) );
        weights_triplets.push_back( WeightMatrix::Triplet( i, bot_i_lft, w_bl ) );
        weights_triplets.push_back( WeightMatrix::Triplet( i, top_i_rgt, w_tr ) );
        weights_triplets.push_back( WeightMatrix::Triplet( i, top_i_lft, w_tl ) );

    }

    W.setFromTriplets(weights_triplets);
}


void Bilinear::print(std::ostream& out) const {
    out << "Bilinear[]";
}


namespace {
static MethodBuilder< Bilinear > __bilinear("bilinear");
}


}  // namespace method
}  // namespace mir

