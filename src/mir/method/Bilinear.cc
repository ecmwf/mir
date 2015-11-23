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

#include <cmath>
#include <algorithm>
#include <string>

#include "eckit/log/Log.h"

#include "atlas/Field.h"
#include "atlas/FunctionSpace.h"
#include "atlas/Mesh.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/grids/ReducedGaussianGrid.h"
#include "atlas/util/ArrayView.h"

#include "mir/util/Compare.h"

using eckit::FloatCompare;

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
//    double left_lon  =   0.;
    for (size_t i = start; i < end; ++i) {

        const double& val = coords[i].data()[LON];
        ASSERT((0. <= val) && (val <= 360.));

        if (val < in || eq(val, in)) {
//            left_lon = val;
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
    const atlas::grids::ReducedGrid* igg = dynamic_cast<const atlas::grids::ReducedGrid*>(&in);
    if (!igg)
        throw eckit::UserError("Bilinear currently only supports Reduced Grids as input");

    const std::vector<long>& lons = igg->points_per_latitude();
    const size_t inpts = igg->npts();

    ASSERT(lons.size());
    ASSERT(lons.front());
    ASSERT(lons.back());


    // pre-allocate matrix entries
    std::vector< WeightMatrix::Triplet > weights_triplets; /* structure to fill-in sparse matrix */
    weights_triplets.reserve( out.npts() );


    // access the input/output fields coordinates
    atlas::ArrayView<double, 2> icoords( in .mesh().nodes().lonlat() );
    atlas::ArrayView<double, 2> ocoords( out.mesh().nodes().lonlat() );


    // check input min/max latitudes (gaussian grids exclude the poles)
    double min_lat = icoords(0, LAT);
    double max_lat = icoords(0, LAT);
    for (size_t i = 1; i < inpts; ++i) {
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
        parallel_south[j] = inpts - i;
    }

//    std::ofstream outfile ("mir.coeffs");
//    outfile.precision(2);

    // interpolate each output point in turn
    const size_t onpts = out.npts();
    for (size_t i = 0; i < onpts; ++i) {

        const double lat = ocoords(i, LAT);
        const double lon = ocoords(i, LON);

        const bool too_much_north = FloatCompare<double>::isStrictlyGreater(lat, max_lat);
        const bool too_much_south = FloatCompare<double>::isStrictlyGreater(min_lat, lat);

        if (too_much_north || too_much_south) {

            ASSERT(too_much_north != too_much_south);

            const std::vector<size_t>& par(too_much_north ? parallel_north : parallel_south);

            const double w = 1. / double(par.size());
            for (std::vector<size_t>::const_iterator j = par.begin(); j != par.end(); ++j)
                weights_triplets.push_back( WeightMatrix::Triplet( i, *j, w ) );

//            outfile << std::fixed
//                    << " " << (size_t) lat * 100 << " "
//                    << w << " "
//                    << w << " "
//                    << w << " "
//                    << w << std::endl;

        } else {

            // find encompassing latitudes ("bottom/top")

            size_t top_n = 0;  // number of points in top latitude line
            size_t bot_n = 0;  // number of points in bottom latitude line

            size_t top_i = 0;  // index of first point in top latitude line
            size_t bot_i = 0;  // index of first point in bottom latitude line

            double top_lat = 0;
            double bot_lat = 0;

            ASSERT(lons.size() >= 2); // at least 2 lines of latitude

            if( FloatCompare<double>::isApproximatelyEqual(max_lat, lat) )
            {
                top_n = lons[0];
                bot_n = lons[1];
                top_i = 0;
                bot_i = top_i + top_n;

            } else {

                if( FloatCompare<double>::isApproximatelyEqual(min_lat, lat) )
                {
                    top_n = lons[ lons.size() - 2 ];
                    bot_n = lons[ lons.size() - 1 ];
                    bot_i = inpts - bot_n;
                    top_i = bot_i - top_n;
                }
                else
                {
                    top_lat = icoords(top_i, LAT);
                    bot_lat = icoords(bot_i, LAT);

                    size_t n = 1;
                    while ( !( bot_lat < lat && FloatCompare<double>::isApproximatelyGreaterOrEqual(top_lat, lat) )
                            && n != lons.size() )
                    {

                        top_n = lons[n - 1];
                        bot_n = lons[n];

                        top_i  = bot_i;
                        bot_i += lons[n - 1];

                        top_lat = icoords(top_i, LAT);
                        bot_lat = icoords(bot_i, LAT);

                        ASSERT(top_lat > bot_lat);

                        ++n;
                    }
                }

            }

            top_lat = icoords(top_i, LAT);
            bot_lat = icoords(bot_i, LAT);

            ASSERT( top_lat > bot_lat );

            // find encompassing longitudes ("left/right")
            // -------------------------------------------

            // set left/right point indices, on the upper latitude
            size_t top_i_lft = 0;
            size_t top_i_rgt = 0;

            left_right_lon_indexes(lon, icoords, top_i, top_i + top_n, top_i_lft, top_i_rgt);

            // set left/right point indices, on the lower latitude
            size_t bot_i_lft = 0;
            size_t bot_i_rgt = 0;

            left_right_lon_indexes(lon, icoords, bot_i, bot_i + bot_n , bot_i_lft, bot_i_rgt);

            // now we have the indices of the input points around the output point


            // bilinear interpolation
            // ----------------------

            ASSERT(bot_i_rgt < inpts);
            ASSERT(bot_i_lft < inpts);
            ASSERT(top_i_rgt < inpts);
            ASSERT(top_i_lft < inpts);

            double tl_lon  = icoords(top_i_lft, LON);
            double tr_lon  = icoords(top_i_rgt, LON);
            double bl_lon  = icoords(bot_i_lft, LON);
            double br_lon  = icoords(bot_i_rgt, LON);

            if( tr_lon < tl_lon ) tr_lon += 360.;
            if( br_lon < bl_lon ) br_lon += 360.;

            // calculate the weights
            double w1 =  lon - tl_lon;
            double w2 =  tr_lon - lon;
            double w3 =  lon - bl_lon;
            double w4 =  br_lon - lon;

            // top and bottom midpoint weights
            double wt = lat - bot_lat;
            double wb = top_lat - lat;

            // weights for the tl, tr, bl, br points
            double w_br =  w3 * wb;
            double w_bl =  w4 * wb;
            double w_tr =  w1 * wt;
            double w_tl =  w2 * wt;

            //            std::cout << " --> LL "
            //                      << lon << " ["
            //                      << tl_lon << "/" << tr_lon << ","
            //                      << bl_lon << "/" << br_lon << "] "
            //                      << lat << " ["
            //                      << top_lat << ","
            //                      << bot_lat << "] : "
            //                      << top_i_lft << " "
            //                      << top_i_rgt << " "
            //                      << bot_i_lft << " "
            //                      << bot_i_rgt << " "
//                      << " *** "
//                      << w1 << " "
//                      << w2 << " "
//                      << w3 << " "
//                      << w4 << " "
//                      << wt << " "
//                      << wb << " "
//                      << std::endl;

            ASSERT( FloatCompare<double>::isApproximatelyGreaterOrEqual(w1, 0.) );
            ASSERT( FloatCompare<double>::isApproximatelyGreaterOrEqual(w2, 0.) );
            ASSERT( FloatCompare<double>::isApproximatelyGreaterOrEqual(w3, 0.) );
            ASSERT( FloatCompare<double>::isApproximatelyGreaterOrEqual(w4, 0.) );
            ASSERT( FloatCompare<double>::isApproximatelyGreaterOrEqual(wt, 0.) );
            ASSERT( FloatCompare<double>::isApproximatelyGreaterOrEqual(wb, 0.) );

            const double sum = w_br + w_bl + w_tr + w_tl;

            ASSERT( sum > 0. );

            w_br /=  sum;
            w_bl /=  sum;
            w_tr /=  sum;
            w_tl /=  sum;

//            outfile << std::fixed
//                    << " " << (size_t) ( lat * 100 ) << " "
//                    << fabs(w_tl) << " "
//                    << fabs(w_tr) << " "
//                    << fabs(w_bl) << " "
//                    << fabs(w_br) << std::endl;

            weights_triplets.push_back( WeightMatrix::Triplet( i, bot_i_rgt, w_br ) );
            weights_triplets.push_back( WeightMatrix::Triplet( i, bot_i_lft, w_bl ) );
            weights_triplets.push_back( WeightMatrix::Triplet( i, top_i_rgt, w_tr ) );
            weights_triplets.push_back( WeightMatrix::Triplet( i, top_i_lft, w_tl ) );

        }

    }

//    outfile.close();

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

