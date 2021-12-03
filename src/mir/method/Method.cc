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

#include "eckit/utils/StringTools.h"

#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"

#if defined(mir_HAVE_OMP)
extern "C" {
void omp_set_num_threads(int);
}
#endif


namespace mir {
namespace method {


Method::Method(const param::MIRParametrisation& params) : parametrisation_(params) {
#if defined(mir_HAVE_OMP)
    int num_threads = 1;
    if (params.get("parallel-omp-num-threads", num_threads)) {
        omp_set_num_threads(num_threads);
    }
#endif
}


Method::~Method() = default;


static util::once_flag once;
static util::recursive_mutex* local_mutex       = nullptr;
static std::map<std::string, MethodFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, MethodFactory*>();
}


MethodFactory::MethodFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("MethodFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MethodFactory::~MethodFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


void MethodFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


Method* MethodFactory::build(std::string& names, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    for (const auto& name : eckit::StringTools::split("/", names)) {
        Log::debug() << "MethodFactory: looking for '" << name << "'" << std::endl;
        auto j = m->find(name);
        if (j != m->end()) {
            names = name;
            return j->second->make(param);
        }
    }

    list(Log::error() << "MethodFactory: unknown '" << names << "', choices are: ");
    throw exception::SeriousBug("MethodFactory: unknown '" + names + "'");
}


}  // namespace method
}  // namespace mir
