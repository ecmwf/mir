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
/// @author Pedro Maciel
/// @date July 2015


#include "mir/method/Bilinear.h"

#include <vector>
#include "eckit/log/BigNum.h"
#include "eckit/log/Log.h"
#include "atlas/array/ArrayView.h"
#include "atlas/array_fwd.h"
#include "atlas/grid.h"
#include "mir/action/context/Context.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/util/MIRGrid.h"
#include "mir/util/Compare.h"
#include "mir/repres/Representation.h"

namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

namespace {


void left_right_lon_indexes(
    const double& in,
    const atlas::array::ArrayView<double, 2>& coords,
    const size_t start,
    const size_t end,
    size_t& left,
    size_t& right) {

    using eckit::geometry::LON;
    using eckit::geometry::LAT;

    eckit::types::CompareApproximatelyEqual<double> eq(10e-10);  //FIXME

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

//----------------------------------------------------------------------------------------------------------------------

Bilinear::Bilinear(const param::MIRParametrisation& param) :
    MethodWeighted(param) {
}


Bilinear::~Bilinear() {
}


const char* Bilinear::name() const {
    return  "bilinear";
}


void Bilinear::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
}


void Bilinear::assemble(WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {

    util::MIRGrid in(rin.grid());
    util::MIRGrid out(rout.grid());


    eckit::Log::debug<LibMir>() << "Bilinear::assemble (input: " << rin << ", output: " << rout << ")" << std::endl;

    using eckit::geometry::LON;
    using eckit::geometry::LAT;


    // NOTE: use bilinear interpolation assuming quasi-regular grid
    // (this assumes the points are oriented north-south)
    // FIXME: proper documentation


    // Ensure the input is a reduced grid, and get the pl array
    const atlas::grid::StructuredGrid igg(in);
    if (!igg) {
        throw eckit::UserError("Bilinear currently only supports Structured grids as input");
    }

    const std::vector<long>& lons = igg.nx();
    const size_t inpts = igg.size();
    const size_t onpts = out.size();

    ASSERT(lons.size());
    ASSERT(lons.front());
    ASSERT(lons.back());


    // pre-allocate matrix entries
    std::vector< WeightMatrix::Triplet > weights_triplets; /* structure to fill-in sparse matrix */
    weights_triplets.reserve( onpts * 4 );


    // access the input/output fields coordinates
    atlas::array::ArrayView<double, 2> icoords = atlas::array::make_view< double, 2 >(in.coordsLonLat());
    atlas::array::ArrayView<double, 2> ocoords = atlas::array::make_view< double, 2 >(out.coordsLonLat());


    // access the input domain
    const util::Domain& idomain = in.domain();


    // check input min/max latitudes (gaussian grids exclude the poles)
    double min_lat = icoords(0, LAT);
    double max_lat = icoords(0, LAT);
    for (size_t i = 1; i < inpts; ++i) {
        const double lat = icoords(i, LAT);
        if (lat < min_lat) min_lat = lat;
        if (lat > max_lat) max_lat = lat;
    }
    ASSERT(min_lat < max_lat);
    eckit::Log::debug<LibMir>() << "Bilinear::assemble max_lat=" << max_lat << ", min_lat=" << min_lat << std::endl;


    // set northern & southern-most parallel point indices
    std::vector<size_t> parallel_north(lons.front());
    std::vector<size_t> parallel_south(lons.back());

    eckit::Log::debug<LibMir>() << "Bilinear::assemble first row: " << lons.front() << std::endl;
    for (long i = 0; i < lons.front(); ++i) {
        parallel_north[i] = size_t(i);
    }

    eckit::Log::debug<LibMir>() << "Bilinear::assemble last row: " << lons.back() << std::endl;
    for (long i = lons.back(), j = 0; i > 0; i--, j++) {
        parallel_south[j] = size_t(inpts - i);
    }

//    std::ofstream outfile ("mir.coeffs");
//    outfile.precision(2);

    // interpolate each output point in turn
    for (size_t i = 0; i < onpts; ++i) {

        const double lat = ocoords(i, LAT);
        const double lon = ocoords(i, LON);

        const bool too_much_north = eckit::types::is_strictly_greater(lat, max_lat);
        const bool too_much_south = eckit::types::is_strictly_greater(min_lat, lat);

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

        } else if (idomain.contains(lat, lon)) {

            // find encompassing latitudes ("bottom/top")

            size_t top_n = 0;  // number of points in top latitude line
            size_t bot_n = 0;  // number of points in bottom latitude line

            size_t top_i = 0;  // index of first point in top latitude line
            size_t bot_i = 0;  // index of first point in bottom latitude line

            double top_lat = 0;
            double bot_lat = 0;

            ASSERT(lons.size() >= 2); // at least 2 lines of latitude

            if( eckit::types::is_approximately_equal<double>(max_lat, lat) ) {

                top_n = lons[0];
                bot_n = lons[1];
                top_i = 0;
                bot_i = top_i + top_n;

            } else if( eckit::types::is_approximately_equal<double>(min_lat, lat) ) {

                top_n = lons[ lons.size() - 2 ];
                bot_n = lons[ lons.size() - 1 ];
                bot_i = inpts - bot_n;
                top_i = bot_i - top_n;

            } else {

                top_lat = icoords(top_i, LAT);
                bot_lat = icoords(bot_i, LAT);

                size_t n = 1;
                while ( !( bot_lat < lat && eckit::types::is_approximately_greater_or_equal(top_lat, lat) )
                        && n != lons.size() ) {

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

            //            eckit::Log::info() << " --> LL "
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

            const double eps = double(std::numeric_limits<float>::epsilon());
            ASSERT( eckit::types::is_approximately_greater_or_equal(w1, 0., eps) );
            ASSERT( eckit::types::is_approximately_greater_or_equal(w2, 0., eps) );
            ASSERT( eckit::types::is_approximately_greater_or_equal(w3, 0., eps) );
            ASSERT( eckit::types::is_approximately_greater_or_equal(w4, 0., eps) );
            ASSERT( eckit::types::is_approximately_greater_or_equal(wt, 0., eps) );
            ASSERT( eckit::types::is_approximately_greater_or_equal(wb, 0., eps) );

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

    // outfile.close();

    eckit::linalg::SparseMatrix M(onpts, inpts, weights_triplets); // build matrix

    W.swap(M);
}


void Bilinear::print(std::ostream& out) const {
    out << "Bilinear[]";
}


namespace {
static MethodBuilder< Bilinear > __bilinear("bilinear");
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

