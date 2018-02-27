/*
 * (C) Copyright 1996- ECMWF.
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
#include "eckit/log/Log.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


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


void Gaussian::correctBoundingBox() {

    // adjust latitudes
    Latitude n = bbox_.north();
    Latitude s = bbox_.south();

    const std::vector<double>& lats = latitudes();
    if (n < lats.back()) {
        n = lats.back();
    } else {
        auto best = std::lower_bound(lats.begin(), lats.end(), n.value(),
                                     [](const Latitude& l1, const Latitude& l2) {
            return !(l1 < l2 || same_with_grib1_accuracy(l1, l2));
        });
        ASSERT(best != lats.end());
        n = *best;
    }

    if (s > lats.front()) {
        s = lats.front();
    } else {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), s.value(),
                                     [](const Latitude& l1, const Latitude& l2) {
            return !(l1 > l2 || same_with_grib1_accuracy(l1, l2));
        });
        ASSERT(best != lats.rend());
        s = *best;
    }

    ASSERT(!same_with_grib1_accuracy(s, n));


    // adjust East
    Longitude e = bbox_.east();

    const Longitude we = e - bbox_.west();
    const Longitude inc = getSmallestIncrement();
    if (same_with_grib1_accuracy(we + inc, Longitude::GLOBE) || we + inc > Longitude::GLOBE) {
        e = bbox_.west() + Longitude::GLOBE - inc;
    }


    // inrform user
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    std::streamsize old = log.precision(12);
    log << "Correcting bounding box from " << bbox_;

    bbox_ = util::BoundingBox(n, bbox_.west(), s, e);

    log << " to " << bbox_ << "." << std::endl;
    log.precision(old);
}


bool Gaussian::includesNorthPole() const {
    return bbox_.north() == latitudes().front();
}


bool Gaussian::includesSouthPole() const {
    return bbox_.south() == latitudes().back();
}


bool Gaussian::isPeriodicWestEast() const {
    return bbox_.east() - bbox_.west() + getSmallestIncrement() == Longitude::GLOBE;
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

