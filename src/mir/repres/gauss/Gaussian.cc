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
    adjustBoundingBoxNorthSouth(bbox_);
}


Gaussian::Gaussian(size_t N, const util::BoundingBox& bbox) :
    Gridded(bbox),
    N_(N) {
    adjustBoundingBoxNorthSouth(bbox_);
}


Gaussian::Gaussian(const param::MIRParametrisation& parametrisation) :
    Gridded(parametrisation) {
    ASSERT(parametrisation.get("N", N_));
    adjustBoundingBoxNorthSouth(bbox_);
}


Gaussian::~Gaussian() {
}


bool Gaussian::sameAs(const Representation& other) const {
    const Gaussian* o = dynamic_cast<const Gaussian*>(&other);
    return o && (N_ == o->N_) && (bbox_ == o->bbox_);
}


void Gaussian::adjustBoundingBoxNorthSouth(util::BoundingBox& bbox) {
    Latitude n = bbox.north();
    Latitude s = bbox.south();
    bool adjustedNorth = false;
    bool adjustedSouth = false;

    const std::vector<double>& lats = latitudes();
    ASSERT(lats.size() >= 2);

    auto range = std::minmax_element(lats.begin(), lats.end());
    ASSERT(*(range.first) < *(range.second));

    if (n > *(range.second)) {
        adjustedNorth = true;
        n = *range.second;
    }
    if (s < *(range.first)) {
        adjustedSouth = true;
        s = *range.first;
    }

    for (const double& l : lats) {
        if (!adjustedNorth && bbox.north().sameWithGrib1Accuracy(l)) {
            adjustedNorth = true;
            n = l;
        }
        if (!adjustedSouth && bbox.south().sameWithGrib1Accuracy(l)) {
            adjustedSouth = true;
            s = l;
        }
        if (adjustedNorth && adjustedSouth) {
            break;
        }
    }

    bbox = util::BoundingBox(n, bbox.west(), s, bbox.east());
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


std::string Gaussian::atlasMeshGenerator() const {
    return "structured";
}


const std::vector<double>& Gaussian::latitudes(size_t N) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(N);
    auto latitudesIt = m->find(N);
    if (latitudesIt == m->end()) {

        // calculate latitudes and insert in known-N-latitudes map
        std::vector<double> latitudes(N * 2);
        atlas::util::gaussian_latitudes_npole_spole(N, latitudes.data());

        m->operator[](N) = latitudes;
        latitudesIt = m->find(N);
    }

    ASSERT(latitudesIt != m->end());
    return (*latitudesIt).second;
}


const std::vector<double>& Gaussian::latitudes() const {
    return latitudes(N_);
}


}  // namespace repres
}  // namespace mir

