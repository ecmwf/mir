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


#include "mir/util/SpectralOrder.h"

#include <map>

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace util {


long SpectralOrder::getTruncationFromGaussianNumber(long) const {
    std::ostringstream os;
    os << "SpectralOrder::getTruncationFromGaussianNumber() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


long SpectralOrder::getGaussianNumberFromTruncation(long) const {
    std::ostringstream os;
    os << "SpectralOrder::getGaussianNumberFromTruncation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


static pthread_once_t once                             = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                       = nullptr;
static std::map<std::string, SpectralOrderFactory*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, SpectralOrderFactory*>();
}


SpectralOrderFactory::SpectralOrderFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("SpectralOrderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


SpectralOrderFactory::~SpectralOrderFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


SpectralOrder* SpectralOrderFactory::build(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    Log::debug() << "SpectralOrderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "SpectralOrderFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("SpectralOrderFactory: unknown '" + name + "'");
    }

    return j->second->make();
}


void SpectralOrderFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace util
}  // namespace mir
