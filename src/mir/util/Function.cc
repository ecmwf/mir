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

#include <map>
#include <ostream>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace util {


static recursive_mutex* local_mutex        = nullptr;
static std::map<std::string, Function*>* m = nullptr;
static once_flag once;
static void init() {
    local_mutex = new recursive_mutex();
    m           = new std::map<std::string, Function*>();
}


Function::Function(const std::string& name) : name_(name) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


Function::~Function() {
    lock_guard<recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


const Function& Function::lookup(const std::string& name) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "Function: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("Function: unknown '" + name + "'");
    }

    return *(j->second);
}


void Function::list(std::ostream& out) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    auto sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace util
}  // namespace mir
