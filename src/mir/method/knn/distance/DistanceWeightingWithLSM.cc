/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "eckit/utils/MD5.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


namespace {


static eckit::Mutex* local_mutex = nullptr;
static std::map< std::string, DistanceWeightingWithLSMFactory* > *m = nullptr;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, DistanceWeightingWithLSMFactory* >();
}


}  // (anonymous namespace)


DistanceWeightingWithLSM::DistanceWeightingWithLSM(const param::MIRParametrisation& parametrisation) {
    std::string name = "nearest-lsm-with-lowest-index";
    parametrisation.get("distance-weighting-with-lsm", name);

    if (!DistanceWeightingWithLSMFactory::has(name)) {
        DistanceWeightingWithLSMFactory::list(eckit::Log::error() << "No DistanceWeightingWithLSMFactory '" << name << "', choices are:\n");
        throw eckit::SeriousBug("No DistanceWeightingWithLSMFactory '" + name + "'");
    }

    method_ = name;
}


const DistanceWeighting* DistanceWeightingWithLSM::distanceWeighting(const param::MIRParametrisation& parametrisation, const lsm::LandSeaMasks& lsm) const {
    return DistanceWeightingWithLSMFactory::build(method_, parametrisation, lsm);
}


bool DistanceWeightingWithLSM::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const DistanceWeightingWithLSM*>(&other);

    // Note: LSM's themselves are not used for this distinction!
    return o && method_ == o->method_;
}


void DistanceWeightingWithLSM::print(std::ostream& out) const {
    out << "DistanceWeightingWithLSM[method=" << method_ << "]";
}


void DistanceWeightingWithLSM::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


DistanceWeightingWithLSMFactory::DistanceWeightingWithLSMFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("DistanceWeightingWithLSMFactory: duplicated DistanceWeightingWithLSM '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


DistanceWeightingWithLSMFactory::~DistanceWeightingWithLSMFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


const DistanceWeighting* DistanceWeightingWithLSMFactory::build(const std::string& name, const param::MIRParametrisation& param, const lsm::LandSeaMasks& lsm) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "DistanceWeightingWithLSMFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "No DistanceWeightingWithLSMFactory '" << name << "', choices are:\n");
        throw eckit::SeriousBug("No DistanceWeightingWithLSMFactory '" + name + "'");
    }

    return j->second->make(param, lsm);
}


void DistanceWeightingWithLSMFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


bool DistanceWeightingWithLSMFactory::has(const std::string& name) {
    return m->find(name) != m->end();
}


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

