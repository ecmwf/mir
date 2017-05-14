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


#include "mir/action/transform/mapping/Resol.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {


static eckit::Mutex* local_mutex = 0;
static std::map< std::string, ResolFactory* > *m = 0;
static pthread_once_t once = PTHREAD_ONCE_INIT;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, ResolFactory* >();
}


} // (anonymous namespace)


Resol::Resol(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {

    std::string spectralMapping = "linear";
    parametrisation.get("spectral-mapping", spectralMapping);

    mapping_.reset(MappingFactory::build(spectralMapping));
    ASSERT(mapping_);
}


Resol::~Resol() {}


size_t Resol::getTruncationFromPointsPerLatitude() const {
    const size_t N = getPointsPerLatitude();
    ASSERT(N);

    return mapping_->getTruncationFromPointsPerLatitude(N);
}


size_t Resol::getPointsPerLatitudeFromTruncation() const {
    const size_t T = getTruncation();
    ASSERT(T);

    return mapping_->getPointsPerLatitudeFromTruncation(T);
}


bool Resol::get(const std::string& name, long& value) const {
    size_t another_value = 0;
    if (get(name, another_value)) {
        value = long(another_value);
        return true;
    }
    return false;
}


bool Resol::get(const std::string& name, size_t& value) const {
    eckit::Log::debug<LibMir>() << "Resol::get(" << name << ")" << std::endl;

    if (name == "truncation") {
        value = getTruncationFromPointsPerLatitude();
        return true;
    }

    if (name == "points-per-latitude") {
        value = getPointsPerLatitudeFromTruncation();
        return true;
    }

    std::ostringstream os;
    os << "Resol::get(" << name <<") not implemented, expected 'truncation' or 'points-per-latitude' (" << *this << ")";
    throw eckit::SeriousBug(os.str());
}


ResolFactory::ResolFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("ResolFactory: duplicate resol: " + name);
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ResolFactory::~ResolFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);
    m->erase(name_);
}


Resol* ResolFactory::build(const std::string& name, const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug< LibMir >() << "Looking for ResolFactory [" << name << "]"
                                  << std::endl;

    std::map< std::string, ResolFactory* >::const_iterator j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No ResolFactory for [" << name << "]"
                            << std::endl;
        eckit::Log::error() << "ResolFactories are:" << std::endl;
        for (j = m->begin(); j != m->end(); ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No ResolFactory called ") + name);
    }

    return (*j).second->make(params);
}


void ResolFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (std::map< std::string, ResolFactory* >::const_iterator j = m->begin();
         j != m->end(); ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


} // namespace mapping
} // namespace transform
} // namespace action
} // namespace mir
