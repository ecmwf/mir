/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "mir/repres/gauss/Gaussian.h"

#include "eckit/exception/Exceptions.h"
#include "atlas/util/GaussianLatitudes.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace repres {


namespace {
void adjustNorthSouth(const std::vector<double>& lats, util::BoundingBox& bbox) {
    ASSERT(lats.size() >= 2);

    Latitude n = bbox.north();
    Latitude s = bbox.south();

    bool adjustedNorth = false;
    bool adjustedSouth = false;

    for (const double& l: lats) {
        if (!adjustedNorth && (n.value() != l) && bbox.north().sameWithGrib1Accuracy(l)) {
            adjustedNorth = true;
            n = l;
        }
        if (!adjustedSouth && (s.value() != l) && bbox.south().sameWithGrib1Accuracy(l)) {
            adjustedSouth = true;
            s = l;
        }
        if (adjustedNorth && adjustedSouth) {
            break;
        }
    }
    if (adjustedNorth || adjustedNorth) {
        eckit::Log::info() << "Gaussian grid BoundingBox (North, South) adjusted to (" << n << ',' << s << ')' << std::endl;
        bbox = util::BoundingBox(n, bbox.west(), s, bbox.east());
    }
}
}  // (anonymous namespace)


Gaussian::Gaussian(size_t N) :
    N_(N) {
    adjustNorthSouth(latitudes(), bbox_);
}


Gaussian::Gaussian(size_t N, const util::BoundingBox &bbox) :
    Gridded(bbox),
    N_(N) {
    adjustNorthSouth(latitudes(), bbox_);
}


Gaussian::Gaussian(const param::MIRParametrisation &parametrisation) :
    Gridded(parametrisation) {
    ASSERT(parametrisation.get("N", N_));
    adjustNorthSouth(latitudes(), bbox_);
}


Gaussian::~Gaussian() {
}


bool Gaussian::sameAs(const Representation& other) const {
    const Gaussian* o = dynamic_cast<const Gaussian*>(&other);
    return o && (N_ == o->N_) && (bbox_ == o->bbox_);
}


bool Gaussian::includesNorthPole() const {
    const std::vector<double>& lats = latitudes();
    ASSERT(lats.size() >= 2);

    return bbox_.north().sameWithGrib1Accuracy(lats.front());
}


bool Gaussian::includesSouthPole() const {
    const std::vector<double>& lats = latitudes();
    ASSERT(lats.size() >= 2);

    return bbox_.south().sameWithGrib1Accuracy(lats.back());
}


std::vector<double> Gaussian::latitudes(size_t N) {
    std::vector<double> latitudes(2 * N);
    atlas::util::gaussian_latitudes_npole_spole(N, &latitudes[0]);
    return latitudes;
}


const std::vector<double>& Gaussian::latitudes() const {
    // This returns the Gaussian latitudes of a GLOBAL field
    if (latitudes_.size() == 0) {
        latitudes_.resize(N_ * 2);
        atlas::util::gaussian_latitudes_npole_spole(N_, &latitudes_[0]);
    }
    return latitudes_;
}


}  // namespace repres
}  // namespace mir

