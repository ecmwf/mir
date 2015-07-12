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
/// @date July 2015


#include "mir/method/Bilinear.h"

#include <algorithm>
#include <string>
#include "atlas/Field.h"
#include "atlas/FunctionSpace.h"
#include "atlas/Mesh.h"
#include "atlas/grids/ReducedGaussianGrid.h"
#include "atlas/util/ArrayView.h"
#include "eckit/log/Log.h"
#include "mir/util/Compare.h"


namespace mir {
namespace method {


namespace {


void left_right_lon_indexes(
    const double& in,
    const atlas::ArrayView<double, 2>& coords,
    const size_t start,
    const size_t end,
    size_t& left,
    size_t& right) {

    using eckit::geometry::LON;
    using eckit::geometry::LAT;
    eckit::FloatApproxCompare< double > eq(10e-10);  //FIXME

    right = start; // take the first if there's a wrap
    left  = start;

    double right_lon = 360.;
    double left_lon  =   0.;
    for (size_t i = start; i < end; ++i) {

        const double& val = coords[i].data()[LON];
        ASSERT((0. <= val) && (val <= 360.));

        if (val < in || eq(val, in)) {
            left_lon = val;
            left     = i;
        } else if (val < right_lon) {
            right_lon = val;
            right     = i;
        }

    }

    ASSERT(left  >= start);
    ASSERT(right >= start);
    ASSERT(right != left);
    ASSERT(eq(coords(left, LAT), coords(right, LAT)));
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
    eckit::FloatApproxCompare< double > eq(10e-10);  //FIXME

    eckit::Log::info() << "Bilinear::assemble " << *this << std::endl;


    // NOTE: use bilinear interpolation assuming quasi-regular grid
    // (this assumes the points are oriented north-south)
    // FIXME: proper documentation


    // Ensure the input is a reduced grid, and get the pl array
    //@todo handle other formats
    const atlas::grids::ReducedGrid* igg = dynamic_cast<const atlas::grids::ReducedGrid*>(&in);
    if (!igg)
        throw eckit::UserError("Bilinear currently only supports Reduced Grids as input");

    const std::vector<long>& lons = igg->points_per_latitude();

    ASSERT(lons.size());
    ASSERT(lons.front());
    ASSERT(lons.back());


    // pre-allocate matrix entries
    std::vector< WeightMatrix::Triplet > weights_triplets; /* structure to fill-in sparse matrix */
    weights_triplets.reserve( out.npts() );


    // access the input/output fields coordinates
    atlas::ArrayView<double, 2> icoords( in .mesh().function_space("nodes").field("lonlat") );
    atlas::ArrayView<double, 2> ocoords( out.mesh().function_space("nodes").field("lonlat") );


    // check input min/max latitudes (gaussian grids exclude the poles)
    double min_lat = icoords(0, LAT);
    double max_lat = icoords(0, LAT);
    for (size_t i = 1; i < in.npts(); ++i) {
        const double lat = icoords(i, LAT);
        if (lat < min_lat) min_lat = lat;
        if (lat > max_lat) max_lat = lat;
    }
    ASSERT(min_lat < max_lat);
    eckit::Log::info() << "Bilinear::assemble max_lat=" << max_lat << ", min_lat=" << min_lat << std::endl;


    // set northern & southern-most parallel point indices
    std::vector<size_t> parallel_north(lons.front());
    std::vector<size_t> parallel_south(lons.back());

    eckit::Log::info() << "Bilinear::assemble first row: " << lons.front() << std::endl;
    for (size_t i = 0; i < lons.front(); ++i) {
        parallel_north[i] = i;
    }

    eckit::Log::info() << "Bilinear::assemble last row: " << lons.back() << std::endl;
    for (size_t i = lons.back(), j = 0; i > 0; i--, j++) {
        parallel_south[j] = in.npts() - i;
    }


    // interpolate each output point in turn
    for (size_t i = 0; i < out.npts(); ++i) {
        const double lat = ocoords(i, LAT);
        const double lon = ocoords(i, LON);

        const bool too_much_north = eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(lat, max_lat);
        const bool too_much_south = eckit::FloatCompare<double>::isApproximatelyGreaterOrEqual(min_lat, lat);

        if (too_much_north || too_much_south) {

            ASSERT(too_much_north != too_much_south);

            const std::vector<size_t>& par(too_much_north ? parallel_north
                                           : parallel_south);
            const double w = 1. / double(par.size());
            for (std::vector<size_t>::const_iterator j = par.begin(); j != par.end(); ++j)
                weights_triplets.push_back( WeightMatrix::Triplet( i, *j, w ) );

        } else {

            // find encompassing latitudes ("bottom/top")
            // ------------------------------------------
            double top_lat = 0.;  // upper/lower latitudes
            double bot_lat = 0.;
            long   top_i = 0;     // upper/lower latitude vector index
            long   bot_i = 0;
            size_t top_n;         // upper/lower latitude vector number of points on the same latitude
            size_t bot_n;

            for (size_t n = 1; n < lons.size(); ++n) {
                top_n = lons[n - 1];
                bot_n = (n == lons.size() ? 0 : lons[n]);

                top_i  = bot_i;
                bot_i += lons[n - 1];
                top_lat = icoords(top_i, LAT);
                bot_lat = icoords(bot_i, LAT);
                ASSERT(top_lat != bot_lat);

                // check output point is on or below the hi latitude
                if (bot_lat < lat && (top_lat > lat || eq(top_lat, lat))) {
                    ASSERT(top_lat > lat || eq(top_lat, lat));
                    ASSERT(bot_lat < lat);
                    ASSERT(!eq(bot_lat, lat));
                    break;
                }
            }

            top_lat = icoords(top_i, LAT);
            bot_lat = icoords(bot_i, LAT);
            ASSERT(top_lat > lat || eq(top_lat, lat));
            ASSERT(bot_lat < lat);
            ASSERT(!eq(bot_lat, lat));


            // find encompassing longitudes ("left/right")
            // -------------------------------------------

            // set left/right point indices, on the upper latitude
            size_t top_i_lft;
            size_t top_i_rgt;
            left_right_lon_indexes(lon, icoords, top_i, top_i + top_n, top_i_lft, top_i_rgt);
            ASSERT(top_i_rgt < bot_i);
            ASSERT(top_i_lft < bot_i);

            // set left/right point indices, on the lower latitude
            size_t bot_i_lft;
            size_t bot_i_rgt;
            left_right_lon_indexes(lon, icoords, bot_i, bot_i + bot_n , bot_i_lft, bot_i_rgt);
            ASSERT(bot_i_rgt < bot_i + bot_n);
            ASSERT(bot_i_lft < bot_i + bot_n);

            // now we have the indices of the input points around the output point


            // bilinear interpolation
            // ----------------------

            double tl_lon  = icoords(top_i_lft, LON);
            double tr_lon  = icoords(top_i_rgt, LON);
            double bl_lon  = icoords(bot_i_lft, LON);
            double br_lon  = icoords(bot_i_rgt, LON);

            // calculate the weights
            double w1 = (tl_lon - lon) / (tl_lon - tr_lon);
            double w2 = 1.0 - w1;
            double w3 = (bl_lon - lon) / (bl_lon - br_lon);
            double w4 = 1.0 - w3;

            // top and bottom midpoint weights
            double wt = (lat - bot_lat) / (top_lat - bot_lat);
            double wb = 1.0 - wt;

            // weights for the tl, tr, bl, br points
            double w_br =  w3 * wb;
            double w_bl =  w4 * wb;
            double w_tr =  w1 * wt;
            double w_tl =  w2 * wt;

            weights_triplets.push_back( WeightMatrix::Triplet( i, bot_i_rgt, w_br ) );
            weights_triplets.push_back( WeightMatrix::Triplet( i, bot_i_lft, w_bl ) );
            weights_triplets.push_back( WeightMatrix::Triplet( i, top_i_rgt, w_tr ) );
            weights_triplets.push_back( WeightMatrix::Triplet( i, top_i_lft, w_tl ) );

        }

    }


    // set sparse matrix
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

