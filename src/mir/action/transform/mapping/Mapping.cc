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


#include "mir/action/transform/mapping/Mapping.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, MappingFactory* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, MappingFactory* >();
}


} // (anonymous namespace)


long Mapping::getTruncationFromPointsPerLatitude(const long&) const {
    std::ostringstream os;
    os << "Mapping::getTruncationFromPointsPerLatitude() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


long Mapping::getPointsPerLatitudeFromTruncation(const long&) const {
    std::ostringstream os;
    os << "Mapping::getPointsPerLatitudeFromTruncation() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


MappingFactory::MappingFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("MappingFactory: duplicate mapping: " + name);
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

    eckit::Log::debug< LibMir >() << "Looking for MappingFactory [" << name << "]"
                                  << std::endl;

    std::map< std::string, MappingFactory* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No MappingFactory for [" << name << "]"
                            << std::endl;
        eckit::Log::error() << "MappingFactories are:" << std::endl;
        for (j = m->begin(); j != m->end(); ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No MappingFactory called ") + name);
    }

    return (*j).second->make();
}


void MappingFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (std::map< std::string, MappingFactory* >::const_iterator j = m->begin();
         j != m->end(); ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


} // namespace mapping
} // namespace transform
} // namespace action
} // namespace mir
