/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/DistanceWeighting.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, DistanceWeightingFactory* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, DistanceWeightingFactory* >();
}


}  // (anonymous namespace)


DistanceWeighting::DistanceWeighting() {
}


DistanceWeighting::~DistanceWeighting() {
}


DistanceWeightingFactory::DistanceWeightingFactory(const std::string& name) :
    name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("DistanceWeightingFactory: duplicated DistanceWeighting '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


DistanceWeightingFactory::~DistanceWeightingFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}


const DistanceWeighting* DistanceWeightingFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "DistanceWeightingFactory: looking for '" << name << "'" << std::endl;

    std::map< std::string, DistanceWeightingFactory* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "No DistanceWeightingFactory '" << name << "', choices are:\n");
        throw eckit::SeriousBug("No DistanceWeightingFactory '" + name + "'");
    }

    return j->second->make(param);
}


void DistanceWeightingFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    std::map< std::string, DistanceWeightingFactory* >::const_iterator j;
    for (j = m->begin(); j != m->end(); ++j) {
        out << sep << j->first;
        sep = ", ";
    }
}


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

