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

#include "mir/lsm/LandSeaMask.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace lsm {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string,LandSeaMaskFactory*> *m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string,LandSeaMaskFactory*>();
}


}  // (anonymous namespace)


LandSeaMask::LandSeaMask(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


LandSeaMask::~LandSeaMask() {
}

//-----------------------------------------------------------------------------


LandSeaMaskFactory::LandSeaMaskFactory(const std::string& name):
    name_(name) {

    pthread_once(&once,init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


LandSeaMaskFactory::~LandSeaMaskFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


LandSeaMask* LandSeaMaskFactory::build(const param::MIRParametrisation& params) {

    pthread_once(&once,init);

    std::string name;

    if(!params.get("logic", name)) {
        throw eckit::SeriousBug("LandSeaMaskFactory cannot get logic");
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, LandSeaMaskFactory*>::const_iterator j = m->find(name);

    eckit::Log::info() << "Looking for LandSeaMaskFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No LandSeaMaskFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "LandSeaMaskFactories are:" << std::endl;
        for(j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No LandSeaMaskFactory called ") + name);
    }

    return (*j).second->make(params);
}


}  // namespace logic
}  // namespace mir

