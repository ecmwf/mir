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
/// @date May 2015


#include "mir/method/Method.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

#include "mir/log/MIR.h"
#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

namespace {

static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex *local_mutex = 0;
static std::map<std::string, MethodFactory *> *m = 0;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, MethodFactory *>();
}

}  // (unnamed namespace)

//----------------------------------------------------------------------------------------------------------------------

Method::Method(const param::MIRParametrisation &params) :
    parametrisation_(params) {
}


Method::~Method() {
}


MethodFactory::MethodFactory(const std::string &name):
    name_(name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MethodFactory::~MethodFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);

}


void MethodFactory::list(std::ostream& out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, MethodFactory *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


Method *MethodFactory::build(const std::string &name, const param::MIRParametrisation &params) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, MethodFactory *>::const_iterator j = m->find(name);

    eckit::Log::trace<MIR>() << "Looking for MethodFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No MethodFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "MethodFactories are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No MethodFactory called ") + name);
    }

    return (*j).second->make(params);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

