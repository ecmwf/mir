/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/repres/gauss/GaussianIterator.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Plural.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/types/Fraction.h"
#include "eckit/utils/MD5.h"
#include "mir/action/misc/AreaCropper.h"
#include "mir/api/MIRJob.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/gauss/reduced/Reduced.h"
#include "mir/repres/gauss/regular/Regular.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/repres/gauss/regular/RotatedGG.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Grib.h"


namespace mir {
namespace repres {
namespace gauss {


GaussianIterator::GaussianIterator(const std::vector<double>& latitudes, const util::Domain& domain, size_t N, size_t countTotal, pl_type pl) :
    latitudes_(latitudes),
    domain_(domain),
    N_(N),
    countTotal_(countTotal),
    pl_(pl),
    Ni_(0),
    Nj_(0),
    west_(domain_.west().fraction()),
    i_(0),
    j_(0),
    count_(0) {

    // position to first latitude and first/last longitude
    k_ = 0;
    while (k_ < latitudes_.size() && domain_.north() < latitudes_[k_]) {
        k_++;
    }
    ASSERT(k_ < latitudes_.size());

#ifdef REGULAR
    // latitudes_ covers the whole globe, but (Ni_,Nj_) cover only the domain
    ASSERT(latitudes_.size() == N * 2);
    //ASSERT(2 <= Ni_ && Ni_ <= N * 4);
    //ASSERT(2 <= Nj_ && Nj_ <= N * 2);

    lon_ = eckit::Fraction(0, 1);
    inc_ = eckit::Fraction(0, 1);
#else

    i_ = 0;
    Ni_ = size_t(pl(k_));
    ASSERT(Ni_);

    lon_ = eckit::Fraction(0);
    inc_ = Longitude::GLOBE.fraction() / Ni_;
#endif
}


GaussianIterator::~GaussianIterator() {
    ASSERT(count_ == countTotal_);
}


void GaussianIterator::print(std::ostream& out) const {
    out << "GaussianIterator["
        << "domain=" << domain_
        << "countTotal=" << countTotal_
        << "N=" << N_
        << "Ni=" << Ni_
        << "Nj=" << Nj_
        << "i=" << i_
        << "j=" << j_
        << "k=" << k_
        << "count=" << count_
        << "]";
}


bool GaussianIterator::next(Latitude& lat, Longitude& lon) {
    while (j_ < Nj_ && i_ < Ni_) {

        ASSERT(j_ + k_ < latitudes_.size());

        lat = latitudes_[j_ + k_];
        lon = lon_;

        i_++;
        lon_ += inc_;

        if (i_ == Ni_) {
            j_++;
            if (j_ < Nj_) {
                i_ = 0;
                Ni_ = size_t(pl_(j_ + k_));
                ASSERT(Ni_);

                lon_ = eckit::Fraction(0);
                inc_ = Longitude::GLOBE.fraction() / Ni_;
            }
        }

        if (domain_.contains(lat, lon)) {
            count_++;
            return true;
        }
    }
    return false;
}


}  // namespace gauss
}  // namespace repres
}  // namespace mir

