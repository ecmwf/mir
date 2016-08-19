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

#include "mir/style/MIRStyle.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace style {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string,MIRStyleFactory*> *m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string,MIRStyleFactory*>();
}


}  // (anonymous namespace)


MIRStyle::MIRStyle(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


MIRStyle::~MIRStyle() {
}

//-----------------------------------------------------------------------------


MIRStyleFactory::MIRStyleFactory(const std::string& name):
    name_(name) {

    pthread_once(&once,init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MIRStyleFactory::~MIRStyleFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


MIRStyle* MIRStyleFactory::build(const param::MIRParametrisation& params) {

    pthread_once(&once,init);

    std::string name;

    if(!params.get("style", name)) {
        throw eckit::SeriousBug("MIRStyleFactory cannot get style");
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, MIRStyleFactory*>::const_iterator j = m->find(name);

    eckit::Log::debug<LibMir>() << "Looking for MIRStyleFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No MIRStyleFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "MIRStyleFactories are:" << std::endl;
        for(j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No MIRStyleFactory called ") + name);
    }

    return (*j).second->make(params);
}


void MIRStyleFactory::list(std::ostream& out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, MIRStyleFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


}  // namespace style
}  // namespace mir

