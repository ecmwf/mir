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

#include <algorithm>
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/api/Atlas.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace repres {


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< size_t, std::vector<double> >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< size_t, std::vector<double> >();
}
}  // (anonymous namespace)


Gaussian::Gaussian(size_t N) :
    N_(N) {
}


Gaussian::Gaussian(size_t N, const util::BoundingBox& bbox) :
    Gridded(bbox),
    N_(N) {
}


Gaussian::Gaussian(const param::MIRParametrisation& parametrisation) :
    Gridded(parametrisation) {
    ASSERT(parametrisation.get("N", N_));
}


Gaussian::~Gaussian() {
}


bool Gaussian::sameAs(const Representation& other) const {
    const Gaussian* o = dynamic_cast<const Gaussian*>(&other);
    return o && (N_ == o->N_) && (bbox_ == o->bbox_);
}


void Gaussian::adjustBoundingBoxNorthSouth(util::BoundingBox& bbox) const {
    const std::vector<double>& lats = latitudes();

    // adjust North
    Latitude n = bbox.north();
    if (n < lats.back()) {
        n = lats.back();
    } else {
        auto best = std::lower_bound(lats.begin(), lats.end(), bbox.north().value(),
                                     [](const Latitude& l1, const Latitude& l2) {
            return !(l1 < l2 || l1.sameWithGrib1Accuracy(l2));
        });
        ASSERT(best != lats.end());
        n = *best;
    }

    // adjust South
    Latitude s = bbox.south();
    if (s > lats.front()) {
        s = lats.front();
    } else {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), bbox.south().value(),
                        [](const Latitude& l1, const Latitude& l2) {
                return !(l1 > l2 || l1.sameWithGrib1Accuracy(l2));
            });
        ASSERT(best != lats.rend());
        s = *best;
    }

    // expect the North/South latitudes to be different
    ASSERT(!s.sameWithGrib1Accuracy(n));

    bbox = util::BoundingBox(n, bbox.west(), s, bbox.east());
}


void Gaussian::adjustBoundingBox(util::BoundingBox& bbox) const {
    adjustBoundingBoxNorthSouth(bbox);
    adjustBoundingBoxEastWest(bbox);
}


bool Gaussian::includesNorthPole() const {
    const std::vector<double>& lats = latitudes();
    return  bbox_.north().sameWithGrib1Accuracy(lats.front()) ||
            bbox_.north() > lats.front();
}


bool Gaussian::includesSouthPole() const {
    const std::vector<double>& lats = latitudes();
    return  bbox_.south().sameWithGrib1Accuracy(lats.back()) ||
            bbox_.south() < lats.back();
}


bool Gaussian::isPeriodicWestEast() const {
    const Longitude we = bbox_.east() - bbox_.west();
    const Longitude inc = getSmallestIncrement();

    return  (we + inc).sameWithGrib1Accuracy(Longitude::GLOBE) ||
            (we + inc > Longitude::GLOBE);
}


std::string Gaussian::atlasMeshGenerator() const {
    return "structured";
}


const std::vector<double>& Gaussian::latitudes(size_t N) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(N);
    auto latitudesIt = m->find(N);
    if (latitudesIt == m->end()) {
        eckit::Timer timer("Gaussian latitudes " + std::to_string(N));

        // calculate latitudes and insert in known-N-latitudes map
        std::vector<double> latitudes(N * 2);
        atlas::util::gaussian_latitudes_npole_spole(N, latitudes.data());

        m->operator[](N) = latitudes;
        latitudesIt = m->find(N);
    }
    ASSERT(latitudesIt != m->end());


    // these are the assumptions we expect from the Gaussian latitudes values
    ASSERT(latitudesIt->second.size() >= 2);
    ASSERT(std::is_sorted(
               latitudesIt->second.begin(),
               latitudesIt->second.end(),
               [](const double& a, const double& b) {
        return a > b;
    } ));

    return (*latitudesIt).second;
}


const std::vector<double>& Gaussian::latitudes() const {
    return latitudes(N_);
}


}  // namespace repres
}  // namespace mir

