/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <string>
#include <algorithm>

#include <boost/progress.hpp>
#include "eckit/geometry/KPoint.h"

#include "atlas/grids/ReducedGaussianGrid.h"
#include "atlas/Tesselation.h"
#include "atlas/grids/Unstructured.h"
#include "atlas/util/ArrayView.h"

#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/Translator.h"

#include "soyuz/method/MethodWeighted.h"
#include "soyuz/method/Bilinear.h"
#include "soyuz/method/PointSearch.h"


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


Bilinear::Bilinear(const MIRParametrisation& param) {
}


Bilinear::~Bilinear() {
}


void Bilinear::assemble(MethodWeighted::Matrix& W) const {
    // FIXME arguments:
    atlas::Grid*      dummy_grid = 0;
    atlas::Grid& in  (*dummy_grid);
    atlas::Grid& out (*dummy_grid);


    using eckit::geometry::LON;
    using eckit::geometry::LAT;

    std::cout << "Bilinear:: compute called " << std::endl;

    atlas::Mesh& i_mesh = in.mesh();
    atlas::Mesh& o_mesh = out.mesh();

    atlas::FunctionSpace& inodes = i_mesh.function_space( "nodes" );
    atlas::FunctionSpace& onodes = o_mesh.function_space( "nodes" );

    atlas::FieldT<double>& ilonlat = inodes.field<double>( "lonlat" );
    atlas::FieldT<double>& olonlat = onodes.field<double>( "lonlat" );

    atlas::ArrayView<double,2> icoords     ( ilonlat );
    atlas::ArrayView<double,2> ocoords     ( olonlat );

    // ReducedGrid involves all grids that can be represented with latitudes and npts_per_lat
    atlas::grids::ReducedGrid* igg = dynamic_cast<atlas::grids::ReducedGrid*>(&in);

    /// @todo we only handle these at the moment
    if (!igg)
        throw eckit::UserError("Bilinear currently only supports Reduced Grids as input");

    // get the longitudes from
    const std::vector<int>& lons = igg->npts_per_lat();

    std::vector< Eigen::Triplet<double> > weights_triplets; /* structure to fill-in sparse matrix */

    // determing the number of output points required
    const size_t out_npts = onodes.shape(0);
    weights_triplets.reserve( out_npts );

    for (unsigned int i = 0; i < out_npts; i++) {
        // get the lat, lon of the output data point required
        double lat = ocoords[i].data()[LAT];
        double lon = ocoords[i].data()[LON];

        // these will hold indices in the input vector of the start of the upper and lower latitudes
        size_t top_i = 0;
        size_t bot_i = 0;

        // iterator over longitudes
        size_t ilon = 0;

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

            double top_lat = icoords[top_i].data()[LAT];

            double bot_lat = icoords[bot_i].data()[LAT];
            ASSERT(top_lat != bot_lat);

            // check output point is on or below the hi latitude
            if (bot_lat < lat && (top_lat > lat || eq(top_lat, lat))) {
                ASSERT(top_lat > lat || eq(top_lat, lat));
                ASSERT(bot_lat < lat);
                ASSERT(!eq(bot_lat, lat));

                break;
            }
        }

        double top_lat = icoords[top_i].data()[LAT];
        double bot_lat = icoords[bot_i].data()[LAT];
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
        ASSERT(eq(icoords[top_i_lft].data()[LAT],  icoords[top_i_rgt].data()[LAT]));

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
        ASSERT(eq(icoords[bot_i_lft].data()[LAT],  icoords[bot_i_rgt].data()[LAT]));

        // we now have the indices of tl, tr, bl, br points around the output point

        double tl_lon  = icoords[top_i_lft].data()[LON];
        double tr_lon  = icoords[top_i_rgt].data()[LON];
        double bl_lon  = icoords[bot_i_lft].data()[LON];
        double br_lon  = icoords[bot_i_rgt].data()[LON];

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

        weights_triplets.push_back( Eigen::Triplet<double>( i, bot_i_rgt, w_br ) );
        weights_triplets.push_back( Eigen::Triplet<double>( i, bot_i_lft, w_bl ) );
        weights_triplets.push_back( Eigen::Triplet<double>( i, top_i_rgt, w_tr ) );
        weights_triplets.push_back( Eigen::Triplet<double>( i, top_i_lft, w_tl ) );

    }

    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
}


namespace {
static MethodBuilder< Bilinear > __bilinear("method.bilinear");
}


}  // namespace method
}  // namespace mir
