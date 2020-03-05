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


#include "mir/util/Function.h"

#include <iostream>
#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"


namespace mir {
namespace util {


static eckit::Mutex* local_mutex           = nullptr;
static std::map<std::string, Function*>* m = nullptr;
static pthread_once_t once                 = PTHREAD_ONCE_INIT;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, Function*>();
}


Function::Function(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


Function::~Function() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


const Function& Function::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "Function: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("Function: unknown '" + name + "'");
    }

    return *(j->second);
}


void Function::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace util
}  // namespace mir
