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

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace repres {
namespace gauss {


GaussianIterator::GaussianIterator(const std::vector<double>& latitudes, const util::BoundingBox& bbox, size_t N, ni_type Ni) :
    latitudes_(latitudes),
    bbox_(bbox),
    N_(N),
    pl_(Ni),
    Ni_(0),
    count_(0) {
    setup();
}


GaussianIterator::GaussianIterator(const std::vector<double>& latitudes, const util::BoundingBox& bbox, size_t N, ni_type Ni, const util::Rotation& rotation) :
    Iterator(rotation),
    latitudes_(latitudes),
    bbox_(bbox),
    N_(N),
    pl_(Ni),
    Ni_(0),
    count_(0) {
    setup();
}


GaussianIterator::~GaussianIterator() {
    eckit::Log::debug<LibMir>() << "GaussianIterator::~GaussianIterator(): count=" << count_ << std::endl;
}


void GaussianIterator::setup() {

    // position to first latitude and first/last longitude
    // NOTE: latitudes_ span the globe, sorted from North-to-South, k_ positions the North
    // NOTE: pl is global
    ASSERT(N_ * 2 == latitudes_.size());

    k_ = 0;
    Nj_ = 0;
    for (auto& lat : latitudes_) {
        if (bbox_.north() < lat) {
            ++k_;
        } else if (bbox_.south() <= lat) {
            ++Nj_;
        } else {
            break;
        }
    }
    ASSERT(Nj_ > 0);

    j_ = 0;
    resetToRow(pl_(j_ + k_));
}


void GaussianIterator::resetToRow(long Ni_globe) {
    ASSERT(Ni_globe > 1);
    inc_ = Longitude::GLOBE.fraction() / Ni_globe;

    const eckit::Fraction w = bbox_.west().fraction();
    auto Nw = (w / inc_).integralPart();
    if (Nw * inc_ < w) {
        Nw += 1;
    }

    const eckit::Fraction e = bbox_.east().fraction();
    auto Ne = (e / inc_).integralPart();
    if (Ne * inc_ > e) {
        Ne -= 1;
    }

    ASSERT(Ne - Nw + 1 > 0);
    Ni_ = size_t(Ne - Nw + 1);

    i_ = 0;
    lon_ = Nw * inc_;
}


void GaussianIterator::print(std::ostream& out) const {
    out << "GaussianIterator["
            "N=" << N_
        << ",bbox=" << bbox_
        << ",Ni=" << Ni_
        << ",Nj=" << Nj_
        << ",i=" << i_
        << ",j=" << j_
        << ",k=" << k_
        << ",count=" << count_
        << "]";
}


bool GaussianIterator::next(Latitude& lat, Longitude& lon) {
    while (j_ < Nj_ && i_ < Ni_) {
        ASSERT(j_ + k_ < latitudes_.size());

        lat = latitudes_[j_ + k_];
        lon = lon_;

        lon_ += inc_;
        if (++i_ == Ni_) {
            if (++j_ < Nj_) {
                resetToRow(pl_(j_ + k_));
            }
        }

        count_++;
        return true;
    }
    return false;
}


}  // namespace gauss
}  // namespace repres
}  // namespace mir

