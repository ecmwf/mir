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


#include "mir/method/Method.h"

#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/StringTools.h"

#include "mir/config/LibMir.h"


namespace mir {
namespace method {


Method::Method(const param::MIRParametrisation& params) : parametrisation_(params) {}


Method::~Method() = default;


static pthread_once_t once                      = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                = nullptr;
static std::map<std::string, MethodFactory*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, MethodFactory*>();
}


MethodFactory::MethodFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("MethodFactory: duplicate '" + name + "'");
    }

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
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


Method* MethodFactory::build(std::string& names, const param::MIRParametrisation& param) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    for (const auto& name : eckit::StringTools::split("/", names)) {
        eckit::Log::debug<LibMir>() << "MethodFactory: looking for '" << name << "'" << std::endl;
        auto j = m->find(name);
        if (j != m->end()) {
            names = name;
            return j->second->make(param);
        }
    }

    list(eckit::Log::error() << "MethodFactory: no valid options in '" << names << "', choices are: ");
    throw eckit::SeriousBug("MethodFactory: no valid options in '" + names + "'");
}


}  // namespace method
}  // namespace mir
