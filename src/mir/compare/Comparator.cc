/*
 * (C) Copyright 1996-2015 ECMWF.
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


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

#include "mir/config/LibMir.h"
#include "mir/compare/Comparator.h"


namespace mir {
namespace compare {

namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, ComparatorFactory *> *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;


static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, ComparatorFactory *>();
}


}  // (anonymous namespace)


Comparator::Comparator(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) :
    parametrisation1_(param1),
    parametrisation2_(param2) {
}


Comparator::~Comparator() {
}


ComparatorFactory::ComparatorFactory(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if(m->find(name) != m->end()) {
        throw eckit::SeriousBug("ComparatorFactory: duplication action: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ComparatorFactory::~ComparatorFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


Comparator *ComparatorFactory::build(const std::string& name, const param::MIRParametrisation& param1, const param::MIRParametrisation& param2) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "Looking for ComparatorFactory [" << name << "]" << std::endl;

    std::map<std::string, ComparatorFactory *>::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No ComparatorFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "ComparatorFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No ComparatorFactory called [") + name + "]");
    }

    return (*j).second->make(param1, param2);
}


void ComparatorFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, ComparatorFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


}  // namespace compare
}  // namespace mir


