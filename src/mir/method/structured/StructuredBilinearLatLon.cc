/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/structured/StructuredBilinearLatLon.h"

#include <memory>
#include <vector>

#include "eckit/types/FloatCompare.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {
namespace structured {


static MethodBuilder<StructuredBilinearLatLon> __method("structured-bilinear-latlon");


StructuredBilinearLatLon::StructuredBilinearLatLon(const param::MIRParametrisation& param) : StructuredMethod(param) {}


StructuredBilinearLatLon::~StructuredBilinearLatLon() = default;


bool StructuredBilinearLatLon::sameAs(const Method& other) const {
    auto o = dynamic_cast<const StructuredBilinearLatLon*>(&other);
    return (o != nullptr) && StructuredMethod::sameAs(other);
}


void StructuredBilinearLatLon::assembleStructuredInput(WeightMatrix& W, const repres::Representation& in,
                                                       const repres::Representation& out) const {

    // NOTE: use bilinear interpolation assuming quasi-regular grid
    // (this assumes the points are oriented north-south)
    // FIXME: proper documentation

    atlas::StructuredGrid gin(in.atlasGrid());
    ASSERT(gin);

    const auto& pl = gin.nx();
    ASSERT(pl.size());
    ASSERT(pl.front());
    ASSERT(pl.back());

    // get input coordinates, checking min/max latitudes (Gaussian grids exclude the poles)
    std::vector<PointLatLon> icoords;
    Latitude min_lat;
    Latitude max_lat;
    getRepresentationPoints(in, icoords, min_lat, max_lat);
    Log::debug() << "StructuredBilinearLatLon::assemble latitude (min,max) = (" << min_lat << ", " << max_lat << ")"
                 << std::endl;

    // set northern & southern-most parallel point indices
    std::vector<size_t> parallel_north(size_t(pl.front()));
    std::vector<size_t> parallel_south(size_t(pl.back()));

    Log::debug() << "StructuredBilinearLatLon::assemble first row: " << pl.front() << std::endl;
    for (size_t i = 0; i < size_t(pl.front()); ++i) {
        parallel_north[i] = i;
    }

    Log::debug() << "StructuredBilinearLatLon::assemble last row: " << pl.back() << std::endl;
    const size_t inpts = in.numberOfPoints();
    for (long i = pl.back(), j = 0; i > 0; i--, j++) {
        parallel_south[size_t(j)] = inpts - size_t(i);
    }

    //    std::ofstream outfile ("mir.coeffs");
    //    outfile.precision(2);

    // fill sparse matrix using triplets (reserve assuming all-quadrilateral interpolations)
    triplet_vector_t triplets;
    size_t nbOutputPoints = out.numberOfPoints();
    triplets.reserve(4 * nbOutputPoints);

    // interpolate each output point in turn
    {
        trace::ProgressTimer progress("Interpolating", nbOutputPoints, {"point"}, Log::debug());

        for (std::unique_ptr<repres::Iterator> it(out.iterator()); it->next();) {
            ++progress;

            auto& p = it->pointUnrotated();
            auto ip = it->index();
            ASSERT(ip < nbOutputPoints);

            const bool too_much_north = p.lat() > max_lat;
            const bool too_much_south = p.lat() < min_lat;

            if (too_much_north || too_much_south) {
                ASSERT(too_much_north != too_much_south);

                const std::vector<size_t>& par(too_much_north ? parallel_north : parallel_south);
                ASSERT(par.size());

                const double w = 1. / double(par.size());
                for (const size_t& jp : par) {
                    triplets.push_back(WeightMatrix::Triplet(ip, jp, w));
                }

                //            outfile << std::fixed
                //                    << " " << (size_t) lat * 100 << " "
                //                    << w << " "
                //                    << w << " "
                //                    << w << " "
                //                    << w << std::endl;
            }
            else {

                // find encompassing latitudes ("bottom/top")

                size_t top_n = 0;  // number of points in top latitude line
                size_t bot_n = 0;  // number of points in bottom latitude line

                size_t top_i = 0;  // index of first point in top latitude line
                size_t bot_i = 0;  // index of first point in bottom latitude line

                Latitude top_lat = 0;
                Latitude bot_lat = 0;

                ASSERT(pl.size() >= 2);  // at least 2 lines of latitude

                if (eckit::types::is_approximately_equal(max_lat.value(), p.lat().value())) {
                    top_n = size_t(pl[0]);
                    bot_n = size_t(pl[1]);
                    top_i = 0;
                    bot_i = top_i + top_n;
                }
                else if (eckit::types::is_approximately_equal(min_lat.value(), p.lat().value())) {
                    top_n = size_t(pl[pl.size() - 2]);
                    bot_n = size_t(pl[pl.size() - 1]);
                    bot_i = inpts - bot_n;
                    top_i = bot_i - top_n;
                }
                else {
                    top_lat = icoords[top_i].lat();
                    bot_lat = icoords[bot_i].lat();

                    size_t n = 1;
                    while (!(bot_lat < p.lat() && (top_lat > p.lat() || eckit::types::is_approximately_equal(
                                                                            top_lat.value(), p.lat().value()))) &&
                           n != pl.size()) {

                        top_n = pl[n - 1];
                        bot_n = pl[n];

                        top_i = bot_i;
                        bot_i += pl[n - 1];

                        top_lat = icoords[top_i].lat();
                        bot_lat = icoords[bot_i].lat();

                        ASSERT(top_lat > bot_lat);

                        ++n;
                    }
                }

                top_lat = icoords[top_i].lat();
                bot_lat = icoords[bot_i].lat();

                ASSERT(top_lat > bot_lat);

                // find encompassing longitudes ("left/right")
                // -------------------------------------------

                // set left/right point indices, on the upper latitude
                size_t top_i_lft = 0;
                size_t top_i_rgt = 0;

                left_right_lon_indexes(p.lon(), icoords, top_i, top_i + top_n, top_i_lft, top_i_rgt);

                // set left/right point indices, on the lower latitude
                size_t bot_i_lft = 0;
                size_t bot_i_rgt = 0;

                left_right_lon_indexes(p.lon(), icoords, bot_i, bot_i + bot_n, bot_i_lft, bot_i_rgt);

                // now we have the indices of the input points around the output point

                // bilinear interpolation
                // ----------------------

                ASSERT(bot_i_rgt < inpts);
                ASSERT(bot_i_lft < inpts);
                ASSERT(top_i_rgt < inpts);
                ASSERT(top_i_lft < inpts);

                Longitude tl_lon = icoords[top_i_lft].lon();
                Longitude tr_lon = icoords[top_i_rgt].lon();
                Longitude bl_lon = icoords[bot_i_lft].lon();
                Longitude br_lon = icoords[bot_i_rgt].lon();

                if (tr_lon < tl_lon) {
                    tr_lon += Longitude::GLOBE.value();
                }
                if (br_lon < bl_lon) {
                    br_lon += Longitude::GLOBE.value();
                }

                // calculate the weights
                Longitude w1 = p.lon() - tl_lon;
                Longitude w2 = tr_lon - p.lon();
                Longitude w3 = p.lon() - bl_lon;
                Longitude w4 = br_lon - p.lon();
                ASSERT(w1 >= 0);
                ASSERT(w2 >= 0);
                ASSERT(w3 >= 0);
                ASSERT(w4 >= 0);

                // top and bottom midpoint weights
                Latitude wt = p.lat() - bot_lat;
                Latitude wb = top_lat - p.lat();
                ASSERT(wt >= 0);
                ASSERT(wb >= 0);

                // weights for the tl, tr, bl, br points
                double w_br = w3.value() * wb.value();
                double w_bl = w4.value() * wb.value();
                double w_tr = w1.value() * wt.value();
                double w_tl = w2.value() * wt.value();

                //            Log::info() << " --> LL "
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

                triplet_vector_t trip(
                    {WeightMatrix::Triplet(ip, bot_i_rgt, w_br), WeightMatrix::Triplet(ip, bot_i_lft, w_bl),
                     WeightMatrix::Triplet(ip, top_i_rgt, w_tr), WeightMatrix::Triplet(ip, top_i_lft, w_tl)});

                // insert local point weights (normalized) into matrix "filler"
                normalise(trip);
                std::copy(trip.begin(), trip.end(), std::back_inserter(triplets));
            }
        }
    }

    // outfile.close();

    // fill sparse matrix
    W.setFromTriplets(triplets);
}


const char* StructuredBilinearLatLon::name() const {
    return "structured-bilinear-latlon";
}


void StructuredBilinearLatLon::hash(eckit::MD5& md5) const {
    StructuredMethod::hash(md5);
}


void StructuredBilinearLatLon::print(std::ostream& out) const {
    out << "StructuredBilinearLatLon[";
    StructuredMethod::print(out);
    out << "]";
}


}  // namespace structured
}  // namespace method
}  // namespace mir
