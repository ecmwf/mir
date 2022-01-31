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


#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"

#include <map>
#include <sstream>

#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


static util::recursive_mutex* local_mutex                         = nullptr;
static std::map<std::string, DistanceWeightingWithLSMFactory*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, DistanceWeightingWithLSMFactory*>();
}


DistanceWeightingWithLSM::DistanceWeightingWithLSM(const param::MIRParametrisation& parametrisation) {
    std::string name = "nearest-lsm-with-lowest-index";
    parametrisation.get("distance-weighting-with-lsm", name);

    if (!DistanceWeightingWithLSMFactory::has(name)) {
        DistanceWeightingWithLSMFactory::list(Log::error()
                                              << "No DistanceWeightingWithLSMFactory '" << name << "', choices are:\n");
        throw exception::SeriousBug("No DistanceWeightingWithLSMFactory '" + name + "'");
    }

    method_ = name;
}


const DistanceWeighting* DistanceWeightingWithLSM::distanceWeighting(const param::MIRParametrisation& parametrisation,
                                                                     const lsm::LandSeaMasks& lsm) const {
    return DistanceWeightingWithLSMFactory::build(method_, parametrisation, lsm);
}


bool DistanceWeightingWithLSM::sameAs(const DistanceWeighting& other) const {
    const auto* o = dynamic_cast<const DistanceWeightingWithLSM*>(&other);

    // Note: LSM's themselves are not used for this distinction!
    return (o != nullptr) && method_ == o->method_;
}


void DistanceWeightingWithLSM::print(std::ostream& out) const {
    out << "DistanceWeightingWithLSM[method=" << method_ << "]";
}


void DistanceWeightingWithLSM::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


DistanceWeightingWithLSMFactory::DistanceWeightingWithLSMFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("DistanceWeightingWithLSMFactory: duplicated DistanceWeightingWithLSM '" + name +
                                    "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


DistanceWeightingWithLSMFactory::~DistanceWeightingWithLSMFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


const DistanceWeighting* DistanceWeightingWithLSMFactory::build(const std::string& name,
                                                                const param::MIRParametrisation& param,
                                                                const lsm::LandSeaMasks& lsm) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "DistanceWeightingWithLSMFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "DistanceWeightingWithLSMFactory: unknown '" << name << "', choices are:\n");
        throw exception::SeriousBug("DistanceWeightingWithLSMFactory: unknown '" + name + "'");
    }

    return j->second->make(param, lsm);
}


void DistanceWeightingWithLSMFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

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
