/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/parser/JSON.h"
#include "eckit/utils/Translator.h"

#include "atlas/Grid.h"
#include "atlas/GridSpec.h"

#include "atlas/grids/LonLatGrid.h"

#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/util/Grib.h"

#include "soyuz/repres/RegularLL.h"


namespace mir {
namespace repres {


RegularLL::RegularLL(const param::MIRParametrisation &parametrisation) {

    eckit::Translator<std::string, double> s2d;
    std::string value;

    ASSERT(parametrisation.get("north", value));
    north_ = s2d(value);

    ASSERT(parametrisation.get("west", value));
    west_ = s2d(value);

    ASSERT(parametrisation.get("south", value));
    south_ = s2d(value);

    ASSERT(parametrisation.get("east", value));
    east_ = s2d(value);

    ASSERT(parametrisation.get("north_south_increment", value));
    north_south_increment_ = s2d(value);

    ASSERT(parametrisation.get("west_east_increment", value));
    west_east_increment_ = s2d(value);

    setNiNj();
}


RegularLL::RegularLL(double north,
                     double west,
                     double south,
                     double east,
                     double north_south_increment,
                     double west_east_increment):
    north_(north),
    west_(west),
    south_(south),
    east_(east),
    north_south_increment_(north_south_increment),
    west_east_increment_(west_east_increment) {
    setNiNj();
}


RegularLL::~RegularLL() {
}


void RegularLL::setNiNj() {
    double ni = (east_ - west_) / west_east_increment_;
    ASSERT(ni > 0);
    ASSERT(long(ni) == ni);
    ni_ = ni + 1;

    double nj = (north_ - south_) / north_south_increment_;
    ASSERT(nj > 0);
    ASSERT(long(nj) == nj);
    nj_ = nj + 1;
}


void RegularLL::print(std::ostream &out) const {
    out << "RegularLL["

        << "north=" << north_
        << ",west=" << west_
        << ",south=" << south_
        << ",east=" << east_

        << ",north_south_increment=" << north_south_increment_
        << ",west_east_increment=" << west_east_increment_

        << ",ni=" << ni_
        << ",nj=" << nj_

        << "]";
}


void RegularLL::fill(grib_info &info) const  {

    // Warning: scanning mode not considered

    info.grid.grid_type = GRIB_UTIL_GRID_SPEC_REGULAR_LL;

    info.grid.Ni = ni_;
    info.grid.Nj = nj_;

    info.grid.iDirectionIncrementInDegrees = west_east_increment_;
    info.grid.jDirectionIncrementInDegrees = north_south_increment_;

    info.grid.longitudeOfFirstGridPointInDegrees = west_;
    info.grid.longitudeOfLastGridPointInDegrees = east_;

    info.grid.latitudeOfFirstGridPointInDegrees = north_;
    info.grid.latitudeOfLastGridPointInDegrees = south_;
}


Representation *RegularLL::crop(double north, double west, double south, double east, const std::vector<double> &in, std::vector<double> &out) const {
    // TODO: An Area class and Increments class
    double n = std::min(north_, north);
    double s = std::max(south_, south);
    double w = std::max(west_, west);
    double e = std::min(east_, east);

    if ( (n != north) && (s != south) && (w != west) && (e != east) ) {
        eckit::Log::warning() << "Crop area not included in field area." << std::endl;
        eckit::Log::warning() << "    Crop request: " << north << "/" << west << "/" << south << "/" << east << std::endl;
        eckit::Log::warning() << "     Actual crop: " << n << "/" << w << "/" << s << "/" << e << std::endl;
    }

    ASSERT( (n - s) >= north_south_increment_ );
    ASSERT( (e - w) >= west_east_increment_ );


    RegularLL *cropped = new RegularLL(n, w, s, e, north_south_increment_, north_south_increment_);
    out = std::vector<double>(cropped->ni() * cropped->ni());

    ASSERT((ni() * nj()) == in.size());
    ASSERT((cropped->ni() * cropped->ni()) == out.size());

    size_t k = 0;
    size_t p = 0;
    double lat = north_;
    for (size_t i = 0; i < ni_; i++, lat -= north_south_increment_) {
        double lon = west_;
        for (size_t j = 0; j < nj_; j++, lon += west_east_increment_) {
            if ( (lat <= n) && (lat >= s) && (lon >= w) && (lon <= e)) {
                ASSERT(k < out.size());
                out[k++] = in[p];
            }
            p++;
        }
    }

    ASSERT(k == out.size());
    ASSERT(p == in.size());

    return cropped;
}


atlas::Grid* RegularLL::atlasGrid() const {
    // TODO: Don't jump in hoops like that
    atlas::Grid *g = atlas::Grid::create(
        atlas::grids::LonLatGrid(west_east_increment_,
            north_south_increment_,
            atlas::grids::LonLatGrid::INCLUDES_POLES).spec());
    return g;
}


size_t RegularLL::frame(std::vector<double> &values, size_t size, double missingValue) const {

    // Could be done better, just a demo
    ASSERT((ni() * nj()) == values.size());
    size_t count = 0;

    size_t k = 0;
    for (size_t j = 0; j < nj_; j++) {
        for (size_t i = 0; i < ni_; i++) {
            if ( !((i < size) || (j < size) || (i >= ni_ - size) || (j >= nj_ - size))) { // Check me, may be buggy
                values[k] = missingValue;
                count++;
            }
            k++;
        }
    }

    ASSERT(k == values.size());
    return count;

}


namespace {
static RepresentationBuilder<RegularLL> regularLL("regular_ll"); // Name is what is returned by grib_api
}


}  // namespace repres
}  // namespace mir

