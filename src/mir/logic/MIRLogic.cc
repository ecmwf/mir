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

#include "mir/logic/MIRLogic.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/action/Action.h"


namespace mir {
namespace logic {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string,MIRLogicFactory*> *m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string,MIRLogicFactory*>();
}


}  // (anonymous namespace)


MIRLogic::MIRLogic(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


MIRLogic::~MIRLogic() {
}


void MIRLogic::add(std::vector<std::auto_ptr< action::Action > >& actions, const std::string& name) const {
    actions.push_back(std::auto_ptr< action::Action >(action::ActionFactory::build(name, parametrisation_)));
}


//-----------------------------------------------------------------------------


MIRLogicFactory::MIRLogicFactory(const std::string& name):
    name_(name) {

    pthread_once(&once,init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MIRLogicFactory::~MIRLogicFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


MIRLogic* MIRLogicFactory::build(const param::MIRParametrisation& params) {

    std::string name;

    if(!params.get("logic", name)) {
        throw eckit::SeriousBug("MIRLogicFactory cannot get logic");
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, MIRLogicFactory*>::const_iterator j = m->find(name);

    eckit::Log::info() << "Looking for MIRLogicFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No MIRLogicFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "MIRLogicFactories are:" << std::endl;
        for(j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No MIRLogicFactory called ") + name);
    }

    return (*j).second->make(params);
}


}  // namespace logic
}  // namespace mir

