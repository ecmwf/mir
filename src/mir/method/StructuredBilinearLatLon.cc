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


#include "mir/method/StructuredBilinearLatLon.h"

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


namespace {
static MethodBuilder< StructuredBilinearLatLon > __method("bilinear");
}


StructuredBilinearLatLon::StructuredBilinearLatLon(const param::MIRParametrisation& param) :
    StructuredMethod(param) {
}


StructuredBilinearLatLon::~StructuredBilinearLatLon() {
}


void StructuredBilinearLatLon::assemble(WeightMatrix& W, const atlas::grid::StructuredGrid& in, const util::MIRGrid& out) const {

    atlas::Grid gout(out);
    ASSERT(gout);


//    eckit::Log::debug<LibMir>() << "StructuredBilinearLatLon::assemble (input: " << in<< ", output: " << out << ")" << std::endl;
    using eckit::geometry::LON;
    using eckit::geometry::LAT;


    // NOTE: use bilinear interpolation assuming quasi-regular grid
    // (this assumes the points are oriented north-south)
    // FIXME: proper documentation



    const std::vector<long>& lons = in.nx();
    const size_t inpts = in.size();
    const size_t onpts = out.size();

    ASSERT(lons.size());
    ASSERT(lons.front());
    ASSERT(lons.back());


    // pre-allocate matrix entries
    std::vector< WeightMatrix::Triplet > weights_triplets; /* structure to fill-in sparse matrix */
    weights_triplets.reserve( onpts * 4 );


    // access the input/output fields coordinates
    // check input min/max latitudes (gaussian grids exclude the poles)
    std::vector<atlas::PointLonLat> icoords(in.size());
    double min_lat = 0.;
    double max_lat = 0.;
    size_t i = 0;
    for (const atlas::PointXY p: in.xy()) {
        double lat = p.y();
        if (!i || lat < min_lat) min_lat = lat;
        if (!i || lat > max_lat) max_lat = lat;
        icoords[i++] = atlas::PointLonLat(p.x(), p.y());
    }
    ASSERT(min_lat < max_lat);
    eckit::Log::debug<LibMir>() << "StructuredBilinearLatLon::assemble max_lat=" << max_lat << ", min_lat=" << min_lat << std::endl;

    atlas::array::ArrayView<double, 2> ocoords = atlas::array::make_view< double, 2 >(out.coordsLonLat());


    // set northern & southern-most parallel point indices
    std::vector<size_t> parallel_north(lons.front());
    std::vector<size_t> parallel_south(lons.back());

    eckit::Log::debug<LibMir>() << "StructuredBilinearLatLon::assemble first row: " << lons.front() << std::endl;
    for (long i = 0; i < lons.front(); ++i) {
        parallel_north[i] = size_t(i);
    }

    eckit::Log::debug<LibMir>() << "StructuredBilinearLatLon::assemble last row: " << lons.back() << std::endl;
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

        } else {

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

                top_lat = icoords[top_i].lat();
                bot_lat = icoords[bot_i].lat();

                size_t n = 1;
                while ( !( bot_lat < lat && eckit::types::is_approximately_greater_or_equal(top_lat, lat) )
                        && n != lons.size() ) {

                    top_n = lons[n - 1];
                    bot_n = lons[n];

                    top_i  = bot_i;
                    bot_i += lons[n - 1];

                    top_lat = icoords[top_i].lat();
                    bot_lat = icoords[bot_i].lat();

                    ASSERT(top_lat > bot_lat);

                    ++n;
                }
            }

            top_lat = icoords[top_i].lat();
            bot_lat = icoords[bot_i].lat();

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

            double tl_lon  = icoords[top_i_lft].lon();
            double tr_lon  = icoords[top_i_rgt].lon();
            double bl_lon  = icoords[bot_i_lft].lon();
            double br_lon  = icoords[bot_i_rgt].lon();

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


const char* StructuredBilinearLatLon::name() const {
    return  "bilinear";
}


void StructuredBilinearLatLon::hash(eckit::MD5& md5) const {
    StructuredMethod::hash(md5);
}


void StructuredBilinearLatLon::print(std::ostream& out) const {
    out << "StructuredBilinearLatLon[]";
}


}  // namespace method
}  // namespace mir

