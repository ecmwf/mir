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


#include "mir/method/knn/distance/DistanceWeighting.h"

#include <map>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::method::knn::distance {


static util::recursive_mutex* local_mutex                  = nullptr;
static std::map<std::string, DistanceWeightingFactory*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, DistanceWeightingFactory*>();
}


DistanceWeighting::DistanceWeighting() = default;


DistanceWeighting::~DistanceWeighting() = default;


DistanceWeightingFactory::DistanceWeightingFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) == m->end()) {
        (*m)[name] = this;
        return;
    }
    throw exception::SeriousBug("DistanceWeightingFactory: duplicated DistanceWeighting '" + name + "'");
}


DistanceWeightingFactory::~DistanceWeightingFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


const DistanceWeighting* DistanceWeightingFactory::build(const std::string& name,
                                                         const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "DistanceWeightingFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "DistanceWeightingFactory: unknown '" << name << "', choices are:\n");
        throw exception::SeriousBug("DistanceWeightingFactory: unknown '" + name + "'");
    }

    return j->second->make(param);
}


void DistanceWeightingFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


bool DistanceWeighting::validateMatrixWeights() const {
    return true;
}


}  // namespace mir::method::knn::distance
