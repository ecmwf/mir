/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date Mar 2017


#include "mir/style/Mapping.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace style {


long Mapping::getTruncationFromGaussianNumber(const long&) const {
    std::ostringstream os;
    os << "Mapping::getTruncationFromGaussianNumber() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


long Mapping::getGaussianNumberFromTruncation(const long&) const {
    std::ostringstream os;
    os << "Mapping::getGaussianNumberFromTruncation() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, MappingFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, MappingFactory* >();
}
}  // (anonymous namespace)


MappingFactory::MappingFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("MappingFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MappingFactory::~MappingFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    m->erase(name_);
}


Mapping* MappingFactory::build(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug<LibMir>() << "MappingFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "MappingFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("MappingFactory: unknown '" + name + "'");
    }

    return (*j).second->make();
}


void MappingFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (auto j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace style
}  // namespace mir
