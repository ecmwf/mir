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
#include "eckit/log/Plural.h"
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


Gaussian::Gaussian(size_t N, const util::BoundingBox& bbox) :
    Gridded(bbox),
    N_(N) {
}


Gaussian::Gaussian(const param::MIRParametrisation& parametrisation) :
    Gridded(parametrisation) {
    ASSERT(parametrisation.get("N", N_));

    correctBoundingBoxFromFile(parametrisation);
}


Gaussian::~Gaussian() {
}


bool Gaussian::sameAs(const Representation& other) const {
    const Gaussian* o = dynamic_cast<const Gaussian*>(&other);
    return o && (N_ == o->N_) && (bbox_ == o->bbox_);
}


void Gaussian::correctBoundingBoxFromFile(const param::MIRParametrisation&) {

    // adjust latitudes
    Latitude n = bbox_.north();
    Latitude s = bbox_.south();

    correctSouthNorth(s, n, true);


    // adjust longitudes
    Longitude e = bbox_.east();
    Longitude w = bbox_.west();

    correctWestEast(w, e, true);


    // set bounding box and inform
    util::BoundingBox newBox = util::BoundingBox(n, w, s, e);

    if (newBox != bbox_) {
        eckit::Channel& log = eckit::Log::debug<LibMir>();
        std::streamsize old = log.precision(12);
        log << "Gaussian::correctBoundingBox: "
            << "\n   " << bbox_
            << "\n > " << newBox
            << std::endl;
        log.precision(old);

        bbox_ = newBox;
    }
}


util::BoundingBox Gaussian::croppedBoundingBox(const util::BoundingBox& bbox) const {

    // adjust latitudes
    Latitude n = bbox.north();
    Latitude s = bbox.south();

    correctSouthNorth(s, n);


    // adjust longitudes
    Longitude e = bbox.east();
    Longitude w = bbox.west();

    correctWestEast(w, e);


    // set bounding box and inform
    util::BoundingBox cropped = util::BoundingBox(n, w, s, e);

    if (cropped != bbox) {
        eckit::Channel& log = eckit::Log::debug<LibMir>();
        std::streamsize old = log.precision(12);
        log << "Gaussian::croppedBoundingBox: "
            << "\n   " << bbox_
            << "\n > " << cropped
            << std::endl;
        log.precision(old);

        return cropped;
    }

    return bbox;
}


Iterator* Gaussian::unrotatedIterator(gauss::GaussianIterator::ni_type Ni) const {
    return new gauss::GaussianIterator(latitudes(), bbox_, N_, Ni);
}


Iterator* Gaussian::rotatedIterator(gauss::GaussianIterator::ni_type Ni, const util::Rotation& rotation) const {
    return new gauss::GaussianIterator(latitudes(), bbox_, N_, Ni, rotation);
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


void Gaussian::validate(const std::vector<double>& values) const {
    const size_t count = numberOfPoints();

    eckit::Log::debug<LibMir>() << "Gaussian::validate checked " << eckit::Plural(values.size(), "value") << ", within domain: " << eckit::BigNum(count) << "." << std::endl;
    ASSERT(values.size() == count);
}


void Gaussian::correctSouthNorth(Latitude& s, Latitude& n, bool grib1) const {
    ASSERT(s < n);

    const std::vector<double>& lats = latitudes();
    ASSERT(!lats.empty());

    if (n < lats.back()) {
        n = lats.back();
    } else {
        auto best = std::lower_bound(lats.begin(), lats.end(), n, grib1 ?
            [](const Latitude& l1, const Latitude& l2) { return !(l1 < l2 || same_with_grib1_accuracy(l1, l2)); } :
            [](const Latitude& l1, const Latitude& l2) { return !(l1 <= l2); } );
        ASSERT(best != lats.end());
        n = *best;
    }

    if (s > lats.front()) {
        s = lats.front();
    } else {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), s, grib1 ?
            [](const Latitude& l1, const Latitude& l2) { return !(l1 > l2 || same_with_grib1_accuracy(l1, l2)); } :
            [](const Latitude& l1, const Latitude& l2) { return !(l1 >= l2); } );
        ASSERT(best != lats.rend());
        s = *best;
    }

    // This is not necessary, but maybe a good idea to require elements of dimensionality 2
    ASSERT(grib1 ? !same_with_grib1_accuracy(s, n)
                 : n != s);
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
        eckit::Timer timer("Gaussian latitudes " + std::to_string(N), eckit::Log::debug<LibMir>());

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

